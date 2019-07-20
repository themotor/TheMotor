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
  template <typename T>
  static size_t GetEventId()
  {
    const static size_t event_id = ++event_count;
    return event_id;
  }

 private:
  static size_t event_count;
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
    assert(handlers_.find(T::template GetEventId<T>()) == handlers_.end());
    handlers_[T::template GetEventId<T>()] =
        [handler{std::move(handler)}](const Event& event) {
          handler(static_cast<const T&>(event));
        };
  }

  template <typename T>
  void Dispatch(const T& e) const
  {
    std::lock_guard<std::mutex> lock(handlers_mu_);
    auto it = handlers_.find(T::template GetEventId<T>());
    if (it == handlers_.end()) return;
    it->second(e);
  }

 private:
  mutable std::mutex handlers_mu_;
  mutable std::unordered_map<size_t, Event::Handler<Event>> handlers_;
};

}  // namespace motor

#endif
