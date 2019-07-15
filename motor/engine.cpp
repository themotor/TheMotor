#include "engine.h"

#include "event.h"
#include "window.h"
namespace motor
{
void Engine::InitializeWindow(WindowOptions opts)
{
  is_running_ = true;
  window_manager_ = WindowPlugin::CreateWindow(std::move(opts));

  // Below part is currently dead code, since CreateWindow will block until
  // destruction. It is here merely to check implementation of event dispatcher
  // compiles.
  Event::Handler<WindowClose> close_handler = [this](const WindowClose& event) {
    is_running_ = false;
  };
  window_manager_->RegisterEventHandler(std::move(close_handler));
}

}  // namespace motor
