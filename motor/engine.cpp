#include "engine.h"

#include "event.h"
#include "window.h"

namespace motor
{
void Engine::InitializeWindow(WindowOptions opts)
{
  is_running_ = true;
  window_manager_ = WindowPlugin::CreateWindow(std::move(opts));

  Event::Handler<WindowClose> close_handler =
      [this](const WindowClose& /*unused*/) { is_running_ = false; };
  window_manager_->RegisterEventHandler(std::move(close_handler));
}

void Engine::MainLoop()
{
  while (is_running_)
  {
    window_manager_->Update();
  }
}

}  // namespace motor
