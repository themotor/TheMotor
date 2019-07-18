#include <cstddef>
#include <memory>

#include "GLFW/glfw3.h"
#include "glog/logging.h"
#include "motor/event.h"
#include "motor/window.h"

namespace motor
{
namespace
{
void GlfwErrorHandler(int /*err_code*/, const char* err_desc)
{
  LOG(FATAL) << "Couldn't initialize GLFWwindow: " << err_desc;
}

class GLFWWindow : public Window
{
 public:
  explicit GLFWWindow(WindowOptions opts) : opts_(std::move(opts))
  {
    glfwSetErrorCallback(GlfwErrorHandler);
    glfwInit();
    handle_ =
        glfwCreateWindow(opts_.width_, opts_.height_, opts_.title_.c_str(),
                         /*monitor=*/nullptr, /*share=*/nullptr);

    glfwSetWindowUserPointer(handle_, this);

    glfwMakeContextCurrent(handle_);
    glfwSetWindowCloseCallback(handle_, CloseCallback);
  }

  ~GLFWWindow() final
  {
    glfwDestroyWindow(handle_);
    glfwTerminate();
  }

  void Update() override
  {
    // TODO(kadircet): Move this into renderer, once we have one ...
    // Just to check it works, should generate a blue window.
    glClearColor(0., 0., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(handle_);
    glfwPollEvents();
  }

 private:
  WindowOptions opts_;
  // Unfortunately we can't use a unique_ptr here, because glfw headers only
  // forward declares GLFWwindow.
  GLFWwindow* handle_;

  static void CloseCallback(GLFWwindow* window)
  {
    LOG(INFO) << "Received close callback";
    auto* glfw_window =
        static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    glfw_window->GetDispatcher().Dispatch(WindowClose());
  }
};

}  // namespace

static WindowPlugin::Set<GLFWWindow> x;

}  // namespace motor
