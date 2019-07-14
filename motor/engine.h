#ifndef _MOTOR_ENGINE_H_
#define _MOTOR_ENGINE_H_

#include <memory>

#include "window.h"

namespace motor
{
class Engine
{
 public:
  void InitializeWindow(WindowOptions opts);

 private:
  std::unique_ptr<Window> window_manager_;
};
}  // namespace motor

#endif
