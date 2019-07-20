#include <cstddef>
#include <memory>

#include "GLFW/glfw3.h"
#include "glog/logging.h"
#include "motor/event.h"
#include "motor/input/input.h"
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
    glfwSetKeyCallback(handle_, KeyCallback);
  }

  ~GLFWWindow() final
  {
    glfwDestroyWindow(handle_);
    glfwTerminate();
  }

  void Update() override
  {
    glfwPollEvents();
    ProcessInputs();
    // TODO(kadircet): Move this into renderer, once we have one ...
    // Just to check it works, should generate a blue window.
    glClearColor(0., 0., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(handle_);
  }

 private:
  WindowOptions opts_;
  // Unfortunately we can't use a unique_ptr here, because glfw headers only
  // forward declares GLFWwindow.
  GLFWwindow* handle_;
  std::vector<input::KeyInput> key_inputs_;

  void ProcessInputs()
  {
    input::InputStateBroadcast broadcast;
    broadcast.keys_ = std::move(key_inputs_);
    // TODO(kadircet): Poll non-event based devices.

    GetDispatcher().Dispatch(broadcast);
  }

  static void CloseCallback(GLFWwindow* window)
  {
    LOG(INFO) << "Received close callback";
    auto* glfw_window =
        static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    glfw_window->GetDispatcher().Dispatch(WindowClose());
  }

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mods)
  {
    VLOG(1) << "Received key callback: " << key << ' ' << scancode << ' ' << ' '
            << action << ' ' << mods << " - " << glfwGetKeyName(key, scancode);
    if (key == GLFW_KEY_UNKNOWN) return;

    auto* glfw_window =
        static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    glfw_window->key_inputs_.push_back(
        {input::KEYBOARD_0, key, action != GLFW_RELEASE});
  }
};

}  // namespace

// Sets GLFWWindow as the window manager when linked in.
static WindowPlugin::Set<GLFWWindow> x;

}  // namespace motor
