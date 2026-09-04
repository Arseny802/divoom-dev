#include "window.h"

#include <ctime>

namespace divoomdev::ics::detail {

time_window compute_window(std::chrono::time_point<std::chrono::system_clock> now,
                           const calendar_settings& settings) {
  auto deadline = now + settings.horizon;

  std::chrono::time_point<std::chrono::system_clock> start = now;
  if (settings.window_from_day_start) {
    time_t now_t = std::chrono::system_clock::to_time_t(now);
    struct tm now_tm;
    localtime_s(&now_tm, &now_t);
    now_tm.tm_hour = 0;
    now_tm.tm_min = 0;
    now_tm.tm_sec = 0;
    time_t day_start_t = mktime(&now_tm);
    if (day_start_t != -1)
      start = std::chrono::system_clock::from_time_t(day_start_t);
  }

  return {start, deadline};
}

bool in_window(std::chrono::time_point<std::chrono::system_clock> tp, const time_window& win) {
  return tp.time_since_epoch().count() != 0 && tp >= win.start && tp <= win.deadline;
}

}  // namespace divoomdev::ics::detail