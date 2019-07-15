#include <GL/gl.h>

#include <cstddef>
#include <memory>

#include "GLFW/glfw3.h"
#include "motor/event.h"
#include "motor/window.h"

namespace motor
{
namespace
{
class GLFWWindow : public Window
{
 public:
  explicit GLFWWindow(WindowOptions opts) : opts_(std::move(opts))
  {
    if (!glfwInit())
    {
      // TODO(kadircet): Log the error.
      exit(EXIT_FAILURE);
    }
    handle_ =
        glfwCreateWindow(opts_.width_, opts_.height_, opts_.title_.c_str(),
                         /*monitor=*/nullptr, /*share=*/nullptr);

    glfwMakeContextCurrent(handle_);
    // TODO(kadircet): Move this into renderer, once we have one ...
    while (true)
    {
      // Just to check it works, should generate a blue window.
      glClearColor(0., 0., 1., 1.);
      glClear(GL_COLOR_BUFFER_BIT);
      glfwSwapBuffers(handle_);
      glfwPollEvents();
      if (glfwWindowShouldClose(handle_))
      {
        dispatcher_.Dispatch(WindowClose());
        break;
      }
    }
  }

  ~GLFWWindow() final
  {
    glfwDestroyWindow(handle_);
    glfwTerminate();
  }

 private:
  WindowOptions opts_;
  // Unfortunately we can't use a unique_ptr here, because glfw headers only
  // forward declares GLFWwindow.
  GLFWwindow* handle_;
};

}  // namespace

static WindowPlugin::Set<GLFWWindow> x;

}  // namespace motor
