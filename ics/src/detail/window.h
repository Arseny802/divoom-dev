#pragma once
#include <chrono>

#include "ics/settings.h"

namespace divoomdev::ics::detail {

/// Event-selection window: [start, deadline]. All bounds come from calendar_settings.
struct time_window {
  std::chrono::time_point<std::chrono::system_clock> start;
  std::chrono::time_point<std::chrono::system_clock> deadline;
};

/// Builds a window around `now` according to settings:
///   - deadline = now + settings.horizon
///   - start = start of the current local day (if window_from_day_start)
///             or `now` otherwise.
time_window compute_window(std::chrono::time_point<std::chrono::system_clock> now,
                           const calendar_settings& settings);

/// Returns true when tp is non-empty and lies within [start, deadline].
bool in_window(std::chrono::time_point<std::chrono::system_clock> tp, const time_window& win);

}  // namespace divoomdev::ics::detail