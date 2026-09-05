#pragma once
#include <string>

namespace divoomdev::ics {

/// Abstraction over a remote calendar source (HTTP, file, mock, ...).
///
/// Keeps the rest of the library network-free and makes it easy to substitute a
/// fake source in unit tests (see MockCalendarSource in tests).
class i_calendar_source {
 public:
  virtual ~i_calendar_source() = default;

  /// Returns raw ICS content for the given URL.
  /// Returns an empty string on failure.
  virtual std::string fetch(const std::string& url) = 0;
};

}  // namespace divoomdev::ics