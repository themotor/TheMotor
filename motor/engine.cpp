#include "engine.h"

#include "window.h"
namespace motor
{
void Engine::InitializeWindow(WindowOptions opts)
{
  window_manager_ = WindowPlugin::CreateWindow(std::move(opts));
}

}  // namespace motor
