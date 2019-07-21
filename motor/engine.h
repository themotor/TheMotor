#ifndef _MOTOR_ENGINE_H_
#define _MOTOR_ENGINE_H_

#include <atomic>
#include <memory>

#include "motor/render/renderer.h"
#include "motor/window.h"

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
  std::atomic<bool> is_running_ = false;
  std::unique_ptr<Renderer> renderer_;
  std::unique_ptr<Window> window_manager_;
};
}  // namespace motor

#endif
