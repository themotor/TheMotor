#ifndef _MOTOR_INPUT_INPUT_H_
#define _MOTOR_INPUT_INPUT_H_

#include <vector>

#include "motor/event.h"

namespace motor::input
{
// TODO(kadircet): the following should come from enums, once we have them...
constexpr const size_t kNumKeys = 512;
constexpr const size_t kNumAxes = 8;

struct KeyInput
{
  size_t device_id_ = 0;
  // TODO(kadircet): Come up with an enum for these.
  int id_ = 0;
  bool is_down_ = false;
};

struct AxisInput
{
  size_t device_id_ = 0;
  // TODO(kadircet): Come up with an enum for these.
  int id_ = 0;
  // In the range [-1., 1]
  float value_ = 0;
};

struct CursorPosition
{
  // Relative to upper left corner of context.
  double x_ = 0;
  double y_ = 0;
};

// This event contains information regarding current state of the input. It is
// not an "on change" event. Senders can ignore inputs that hasn't change state.
// If the receiver cares about "on change" events, it must handle the
// bookkeeping on its own.
struct InputStateBroadcast : public Event
{
  std::vector<KeyInput> keys_;
  std::vector<AxisInput> axes_;
  CursorPosition cursor_;
};

}  // namespace motor::input

#endif
