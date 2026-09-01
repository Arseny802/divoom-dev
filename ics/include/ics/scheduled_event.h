#pragma once
#include "common/event.h"
#include <vector>

namespace divoomdev::ics {

struct scheduled_event : common::event {
  enum class repeat_t {
    ONCE,
    DAYLY,
    WEEKLY,
    MONTHLY,
    BYDAY,
  } repeat = repeat_t::ONCE;

  enum dayly_periodic : uint8_t {
    NONE = 0,
    MONDAY = 1,
    TUESDAY = 2,
    WEDNSEDAY = 4,
    THURSDAY = 8,
    FRIDAY = 16,
    SATURADY = 32,
    SUNDARY = 64
  } byday_mask = NONE;
};
using scheduled_event_list = std::vector<scheduled_event>;

}  // namespace divoomdev::ics
