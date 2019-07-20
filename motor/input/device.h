#ifndef _MOTOR_INPUT_DEVICE_H_
#define _MOTOR_INPUT_DEVICE_H_

#include "motor/event.h"

namespace motor::input
{
struct DeviceConfigChange : public Event
{
  size_t device_id_ = 0;
  enum
  {
    DISCONNECTED = 0,
    CONNECTED,
  } status_ = DISCONNECTED;
};

}  // namespace motor::input

#endif
