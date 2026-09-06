#pragma once
#include <chrono>
#include <string>

#include "ics/source.h"

namespace divoomdev::ics {

/// Real HTTP implementation of i_calendar_source backed by cpr/libcurl.
///
/// Features:
/// - Configurable timeout and retry count with exponential backoff.
/// - Graceful degradation: returns empty string on persistent failure.
/// - Detailed error logging for diagnostics.
class http_calendar_source final : public i_calendar_source {
 public:
  /// Configuration for HTTP fetching.
  struct http_options {
    /// Connect and read timeout per attempt (default: 15 seconds).
    std::chrono::seconds timeout{15};
    /// Number of retry attempts on transient failure (default: 3).
    int retries{3};
    /// Base delay between retries in milliseconds (default: 500ms).
    std::chrono::milliseconds retry_delay{500};
    /// Disable TLS host and peer verification (default: true for Outlook compatibility).
    bool ssl_verify{false};
  };

  /// Creates a source with default options.
  http_calendar_source();

  /// Creates a source with custom options.
  explicit http_calendar_source(http_options opts);

  /// Performs an HTTPS GET with retries and returns the body.
  /// Returns an empty string on failure (logged).
  std::string fetch(const std::string& url) override;

  /// Returns current options (useful for diagnostics).
  const http_options& options() const noexcept { return opts_; }

 private:
  http_options opts_;
};

}  // namespace divoomdev::ics