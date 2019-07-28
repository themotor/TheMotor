#ifndef _MOTOR_INPUT_INPUT_H_
#define _MOTOR_INPUT_INPUT_H_

#include <vector>

#include "motor/event.h"

namespace motor::input
{
// TODO(kadircet): the following should come from enums, once we have them...
constexpr const size_t kNumKeys = 256;
constexpr const size_t kNumAxes = 8;

enum DeviceType
{
  UNKNOWN_DEVICE = 0,
  // TODO(kadircet): Handle multiple devices of same type.
  KEYBOARD_0,
  MOUSE_0,
  GAMEPAD_0,
};

struct KeyInput
{
  DeviceType device_id_ = UNKNOWN_DEVICE;
  // TODO(kadircet): Come up with an enum for these.
  int id_ = 0;
  bool is_down_ = false;
};

struct AxisInput
{
  DeviceType device_id_ = UNKNOWN_DEVICE;
  // TODO(kadircet): Come up with an enum for these.
  int id_ = 0;
  // In the range [-1., 1]
  float value_ = 0;
};

// This event contains information regarding current state of the input. It is
// not an "on change" event. Senders can ignore inputs that hasn't change state.
// If the receiver cares about "on change" events, it must handle the
// bookkeeping on its own.
struct InputStateBroadcast : public Event
{
  std::vector<KeyInput> keys_;
  std::vector<AxisInput> axes_;
};

}  // namespace motor::input

#endif
