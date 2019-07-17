#ifndef _MOTOR_ENGINE_H_
#define _MOTOR_ENGINE_H_

#include <atomic>
#include <memory>

#include "window.h"

namespace motor
{
class Engine
{
 public:
  Engine() = default;
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&) = delete;
  ~Engine() = default;

  void InitializeWindow(WindowOptions opts);
  void MainLoop();

 private:
  std::unique_ptr<Window> window_manager_;
  std::atomic<bool> is_running_ = false;
};
}  // namespace motor

#endif
