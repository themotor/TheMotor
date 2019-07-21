#ifndef _MOTOR_WINDOW_H_
#define _MOTOR_WINDOW_H_

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "event.h"
#include "plugin.h"

namespace motor
{
struct WindowOptions
{
  std::string title_;

  // Denotes number of pixels in frame buffer.
  size_t width_ = 0;
  size_t height_ = 0;

  bool is_full_screen_ = false;
};

class Window
{
 public:
  virtual ~Window() = default;

  template <typename T>
  void RegisterEventHandler(Event::Handler<T> handler)
  {
    dispatcher_.Set<T>(std::move(handler));
  }

  void SetOptions(WindowOptions opts);
  virtual void CreateWindow() = 0;

  virtual void Update() = 0;

 protected:
  const EventDispatcher& GetDispatcher() const { return dispatcher_; }
  const WindowOptions& GetOptions() const { return opts_; }

 private:
  EventDispatcher dispatcher_;
  WindowOptions opts_;
};

using WindowPlugin = SinglePluginRegistry<Window>;

}  // namespace motor
#endif
