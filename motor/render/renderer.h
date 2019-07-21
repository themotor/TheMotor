#ifndef _MOTOR_RENDER_RENDERER_H_
#define _MOTOR_RENDER_RENDERER_H_

#include "motor/plugin.h"

namespace motor
{
class Renderer
{
 public:
  virtual ~Renderer() = default;

  virtual void Render() = 0;
};

using RenderPlugin = SinglePluginRegistry<Renderer>;

}  // namespace motor
#endif
