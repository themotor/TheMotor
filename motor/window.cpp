#include "window.h"

namespace motor
{
void Window::SetOptions(WindowOptions opts) { opts_ = std::move(opts); }

}  // namespace motor
