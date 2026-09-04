#pragma once
#include "ics/source.h"

namespace divoomdev::ics {

/// Real HTTP implementation of i_calendar_source backed by cpr/libcurl.
class http_calendar_source final : public i_calendar_source {
 public:
  /// Performs an HTTPS GET (TLS verification disabled) and returns the body.
  std::string fetch(const std::string& url) override;
};

}  // namespace divoomdev::ics