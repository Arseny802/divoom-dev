#pragma once
#include <chrono>
#include <string>
#include <vector>

#include "ics/scheduled_event.h"
#include "ics/settings.h"

namespace divoomdev::ics::detail {

/// Core calendar pipeline: unfold -> parse VEVENT -> expand recurrence ->
/// apply overrides -> filter by window -> sort.
///
/// Takes an explicit "now" and the consumer's calendar_settings so that tests can
/// pin a deterministic moment and adjust the selection window.
scheduled_event_list parse_calendar_at(const std::string& ics_content,
                                       std::chrono::time_point<std::chrono::system_clock> now,
                                       const calendar_settings& settings);

}  // namespace divoomdev::ics::detail