#include "engine.h"

#include "event.h"
#include "glog/logging.h"
#include "input/input.h"
#include "window.h"

namespace motor
{
namespace
{
void InputStateHandler(const input::InputStateBroadcast& state)
{
  for (const input::KeyInput& key_inp : state.keys_)
  {
    VLOG(1) << "Device " << key_inp.device_id_ << ", Key " << key_inp.id_
            << " is_down: " << key_inp.is_down_;
  }
  for (const input::AxisInput& axis_inp : state.axes_)
  {
    VLOG(1) << "Device " << axis_inp.device_id_ << ", Axis " << axis_inp.id_
            << " value: " << axis_inp.value_;
  }
}

}  // namespace

void Engine::InitializeWindow(WindowOptions opts)
{
  is_running_ = true;
  window_manager_ = WindowPlugin::Create();
  window_manager_->SetOptions(std::move(opts));
  window_manager_->CreateWindow();

  Event::Handler<WindowClose> close_handler =
      [this](const WindowClose& /*unused*/) {
        VLOG(1) << "Engine received close";
        is_running_ = false;
      };
  window_manager_->RegisterEventHandler(std::move(close_handler));

  window_manager_->RegisterEventHandler(
      static_cast<Event::Handler<input::InputStateBroadcast>>(
          InputStateHandler));
}

void Engine::MainLoop()
{
  while (is_running_)
  {
    window_manager_->Update();
  }
}

}  // namespace motor
