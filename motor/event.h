#ifndef _MOTOR_EVENT_H_
#define _MOTOR_EVENT_H_

#include <functional>
#include <mutex>
#include <unordered_map>

namespace motor
{
struct Event
{
  virtual ~Event() = default;
  template <typename T>
  using Handler = std::function<void(const T&)>;

  // Note that this only guarantees uniqueness of Id, they might still change
  // from compilation to compilation. But they will be same within the same
  // compilation.
  static size_t GetEventId();
};

struct WindowClose : public Event
{
};

class EventDispatcher
{
 public:
  template <typename T>
  void Set(Event::Handler<T> handler)
  {
    static_assert(std::is_base_of<Event, T>::value,
                  "Set expects a class derived from motor::Event");
    std::lock_guard<std::mutex> lock(handlers_mu_);
    handlers_[T::GetEventId()] = std::bind(
        [](Event::Handler<T> handler, const Event& event) {
          handler(static_cast<const T&>(event));
        },
        std::move(handler), std::placeholders::_1);
  }

  void Dispatch(const Event& e) const;

 private:
  mutable std::mutex handlers_mu_;
  mutable std::unordered_map<size_t, Event::Handler<Event>> handlers_;
};

}  // namespace motor

#endif
