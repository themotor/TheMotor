#include "event.h"

#include <cstddef>
#include <mutex>
#include <type_traits>

namespace motor
{
namespace
{
static size_t GenerateEventId()
{
  static size_t event_count = 0;
  return ++event_count;
}

}  // namespace

size_t Event::GetEventId()
{
  const static size_t event_id = GenerateEventId();
  return event_id;
}

void EventDispatcher::Dispatch(const Event& e) const
{
  std::lock_guard<std::mutex> lock(handlers_mu_);
  auto it = handlers_.find(e.GetEventId());
  if (it == handlers_.end()) return;
  it->second(e);
}
}  // namespace motor
