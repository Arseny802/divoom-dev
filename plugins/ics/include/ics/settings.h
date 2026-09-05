#pragma once
#include <chrono>

namespace divoomdev::ics {

/// Configuration for event-selection windows.
/// All time intervals used by the library are driven by these settings, so the
/// consumer controls the look-ahead horizon without touching the parsing logic.
struct calendar_settings {
  /// Look-ahead horizon for event selection (default: 48 hours).
  std::chrono::hours horizon{48};

  /// When true, the lower bound of the window is the start of the current local
  /// day, so events that already started today are not dropped (important for
  /// recurrence expansion). Otherwise the lower bound is the current moment.
  bool window_from_day_start{true};
};

}  // namespace divoomdev::ics