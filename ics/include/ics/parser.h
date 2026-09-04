#pragma once
#include <chrono>
#include <string>

#include "ics/scheduled_event.h"
#include "ics/settings.h"

namespace divoomdev::ics {

/// High-level parser facade: turns raw ICS content into a list of events.
///
/// The selection window is fully driven by calendar_settings (see settings.h),
/// so the look-ahead horizon is configurable by the consumer. Instances are
/// lightweight and stateless besides the settings, and `parse_at` lets tests
/// pin an explicit "now" for deterministic behaviour.
class calendar_parser {
 public:
  explicit calendar_parser(calendar_settings settings = {});

  /// Parses content using the current wall-clock time as the window anchor.
  scheduled_event_list parse(const std::string& ics_content) const;

  /// Parses content relative to an explicit moment (used by tests).
  scheduled_event_list parse_at(const std::string& ics_content,
                                std::chrono::time_point<std::chrono::system_clock> now) const;

  void set_settings(const calendar_settings& settings);
  const calendar_settings& settings() const noexcept;

 private:
  calendar_settings settings_;
};

}  // namespace divoomdev::ics
