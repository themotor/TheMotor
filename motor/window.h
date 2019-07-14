#ifndef _MOTOR_WINDOW_H_
#define _MOTOR_WINDOW_H_

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

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
};

class WindowPlugin
{
 public:
  template <typename T>
  class Set
  {
   private:
    static std::unique_ptr<Window> CreateWindow(WindowOptions opts)
    {
      return std::make_unique<T>(std::move(opts));
    }

   public:
    Set()
    {
      assert(create_window == nullptr &&
             "A window plugin has already been registered!");
      create_window = CreateWindow;
    }
  };

  static std::unique_ptr<Window> CreateWindow(WindowOptions opts);

 private:
  static std::unique_ptr<Window> (*create_window)(WindowOptions opts);
};

}  // namespace motor
#endif
