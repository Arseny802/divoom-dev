#pragma once
#include "common/event.h"
#include <set>
#include <string>
#include <vector>


namespace divoomdev::ics {

struct scheduled_event : common::event {
  enum class repeat_t {
    ONCE,
    DAILY,
    WEEKLY,
    MONTHLY,
    YEARLY,
  } repeat = repeat_t::ONCE;

  enum day_periodic : uint8_t {
    NONE = 0,
    MONDAY = 1,
    TUESDAY = 2,
    WEDNESDAY = 4,
    THURSDAY = 8,
    FRIDAY = 16,
    SATURDAY = 32,
    SUNDAY = 64
  } byday_mask = NONE;

  int recurrence_interval = 1;

  std::chrono::time_point<std::chrono::system_clock> recurrence_end;

  std::set<std::string> exdates;  // UID + local time string для исключений
};
using scheduled_event_list = std::vector<scheduled_event>;

}  // namespace divoomdev::ics
