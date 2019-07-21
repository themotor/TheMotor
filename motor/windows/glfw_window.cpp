#include <cstddef>
#include <memory>
#include <vector>

#include "GLFW/glfw3.h"
#include "glog/logging.h"
#include "motor/event.h"
#include "motor/input/device.h"
#include "motor/input/input.h"
#include "motor/window.h"

namespace motor
{
namespace
{
constexpr const size_t kKeyboardId = 0;
constexpr const size_t kMouseId = 1;
constexpr const size_t kFirstJoystickId = 2;

void GlfwErrorHandler(int /*err_code*/, const char* err_desc)
{
  LOG(FATAL) << "Couldn't initialize GLFWwindow: " << err_desc;
}

void AppendGamepadStates(input::InputStateBroadcast* broadcast)
{
  GLFWgamepadstate gamepad_state;
  for (size_t joystick_id = GLFW_JOYSTICK_1; joystick_id <= GLFW_JOYSTICK_LAST;
       ++joystick_id)
  {
    // TODO(kadircet): Support joysticks without a gamepad mapping.
    if (!glfwGetGamepadState(joystick_id, &gamepad_state)) continue;

    for (int key_id = GLFW_GAMEPAD_BUTTON_A; key_id <= GLFW_GAMEPAD_BUTTON_LAST;
         ++key_id)
    {
      input::KeyInput& key_inp = broadcast->keys_.emplace_back();
      key_inp.device_id_ = joystick_id;
      key_inp.id_ = key_id;
      key_inp.is_down_ = gamepad_state.buttons[key_id] == GLFW_PRESS;
    }

    for (int axis_id = GLFW_GAMEPAD_AXIS_LEFT_X;
         axis_id <= GLFW_GAMEPAD_AXIS_LAST; ++axis_id)
    {
      input::AxisInput& axis_inp = broadcast->axes_.emplace_back();
      axis_inp.device_id_ = joystick_id;
      axis_inp.id_ = axis_id;
      axis_inp.value_ = gamepad_state.axes[axis_id];
    }
  }
}

class GLFWWindow : public Window
{
 public:
  GLFWWindow()
  {
    glfwSetErrorCallback(GlfwErrorHandler);
    glfwInit();
  }

  void CreateWindow() override
  {
    const WindowOptions& opts = GetOptions();
    handle_ = glfwCreateWindow(opts.width_, opts.height_, opts.title_.c_str(),
                               /*monitor=*/nullptr, /*share=*/nullptr);

    glfwSetWindowUserPointer(handle_, this);
    glfwSetWindowCloseCallback(handle_, CloseCallback);
    glfwMakeContextCurrent(handle_);

    InitializeInputs();
  }

  ~GLFWWindow() final
  {
    glfwDestroyWindow(handle_);
    glfwTerminate();
  }

  void Update() override
  {
    key_inputs_.clear();
    glfwPollEvents();
    BroadcastInputState();

    // TODO(kadircet): Move this into renderer, once we have one ...
    // Just to check it works, should generate a blue window.
    glClearColor(0., 0., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(handle_);
  }

 private:
  // Unfortunately we can't use a unique_ptr here, because glfw headers only
  // forward declares GLFWwindow.
  GLFWwindow* handle_;
  std::vector<input::KeyInput> key_inputs_;

  void InitializeInputs()
  {
    input::DeviceConfigChange config_event;
    config_event.status_ = input::DeviceConfigChange::CONNECTED;
    // Initial keybaord and mouse is always registered and we won't receive
    // callbacks for those so dispatch those now.
    for (size_t device_type : {kKeyboardId, kMouseId})
    {
      config_event.device_id_ = device_type;
      GetDispatcher().Dispatch(config_event);
    }
    glfwSetKeyCallback(handle_, KeyCallback);
    glfwSetMouseButtonCallback(handle_, MouseButtonCallback);

    // Check for connected joysticks, since we won't receive a callback if it is
    // already registered.
    for (size_t i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; ++i)
    {
      if (!glfwJoystickPresent(i)) continue;
      config_event.device_id_ = kFirstJoystickId + i;
      GetDispatcher().Dispatch(config_event);
    }
    glfwSetJoystickCallback(JoystickCallback);
  }

  void BroadcastInputState()
  {
    input::InputStateBroadcast broadcast;
    broadcast.keys_ = std::move(key_inputs_);
    AppendGamepadStates(&broadcast);
    glfwGetCursorPos(handle_, &broadcast.cursor_.x_, &broadcast.cursor_.y_);

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
    VLOG(2) << "Received key callback: " << key << ' ' << scancode << ' '
            << action << ' ' << mods << " - " << glfwGetKeyName(key, scancode);
    if (key == GLFW_KEY_UNKNOWN) return;
    if (action == GLFW_REPEAT) return;

    auto* glfw_window =
        static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    glfw_window->key_inputs_.push_back(
        {kKeyboardId, key, action == GLFW_PRESS});
  }

  static void MouseButtonCallback(GLFWwindow* window, int button, int action,
                                  int mods)
  {
    VLOG(2) << "Received mouse button callback: " << button << ' ' << action
            << ' ' << mods;

    auto* glfw_window =
        static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    glfw_window->key_inputs_.push_back(
        {kMouseId, button, action == GLFW_PRESS});
  }

  static void JoystickCallback(int joystic_id, int event)
  {
    VLOG(1) << "Received " << event << " for " << joystic_id
            << " with name: " << glfwGetJoystickName(joystic_id);
    auto* glfw_window = static_cast<GLFWWindow*>(
        glfwGetMonitorUserPointer(glfwGetPrimaryMonitor()));

    input::DeviceConfigChange config_event;
    config_event.status_ = event == GLFW_CONNECTED
                               ? input::DeviceConfigChange::CONNECTED
                               : input::DeviceConfigChange::DISCONNECTED;
    config_event.device_id_ = kFirstJoystickId + joystic_id;
    glfw_window->GetDispatcher().Dispatch(config_event);
  }
};

}  // namespace

// Sets GLFWWindow as the window manager when linked in.
static WindowPlugin::Set<GLFWWindow> x;

}  // namespace motor
