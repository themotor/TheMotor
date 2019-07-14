#include "window.h"

namespace motor
{
std::unique_ptr<Window> (*WindowPlugin::create_window)(WindowOptions opts) =
    nullptr;

std::unique_ptr<Window> WindowPlugin::CreateWindow(WindowOptions opts)
{
  assert(create_window != nullptr && "No window plugin has been registered");
  return create_window(std::move(opts));
}

}  // namespace motor
