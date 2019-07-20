#include "event.h"

#include <cstddef>
#include <mutex>
#include <type_traits>

namespace motor
{
size_t Event::event_count = 0;
}  // namespace motor
