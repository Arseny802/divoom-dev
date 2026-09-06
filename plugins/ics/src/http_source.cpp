#include "ics/http_source.h"

#include <thread>

#include "cpr/cpr.h"

namespace divoomdev::ics {

namespace {

/// Returns true if the error is transient and worth retrying.
bool is_transient_error(const std::string& msg) {
  // SSL/TLS errors, connection timeouts, connection resets, server errors
  const char* transient_patterns[] = {
      "SSL",
      "ssl",
      "TLS",
      "tls",
      "timeout",
      "Timeout",
      "timed out",
      "connection refused",
      "Connection refused",
      "connection reset",
      "Connection reset",
      "bad record mac",
      "decryption failed",
      "certificate",
      "Certificate",
      "502",
      "503",
      "504",  // common proxy/server error codes in message
  };
  for (const auto* pat: transient_patterns) {
    if (msg.find(pat) != std::string::npos)
      return true;
  }
  return false;
}

}  // namespace

http_calendar_source::http_calendar_source() = default;

http_calendar_source::http_calendar_source(http_options opts): opts_(std::move(opts)) { }

std::string http_calendar_source::fetch(const std::string& url) {
  cpr::Parameters parameters;
  cpr::SslOptions ssl_options;
  ssl_options.verify_host = opts_.ssl_verify ? true : false;
  ssl_options.verify_peer = opts_.ssl_verify ? true : false;

  std::string last_error;
  int last_status = 0;

  for (int attempt = 0; attempt <= opts_.retries; ++attempt) {
    if (attempt > 0) {
      // Exponential backoff: delay * 2^(attempt-1)
      auto delay = opts_.retry_delay * (1 << (attempt - 1));
      hlog()->warn("[ICS]: Retry {}/{} for {} after {}ms delay", attempt, opts_.retries, url, delay.count());
      std::this_thread::sleep_for(delay);
    }

    hlog()->debug("[ICS]: Fetch attempt {}/{} {}", attempt + 1, opts_.retries + 1, url);

    cpr::Response response = cpr::Get(cpr::Url{url},
                                      parameters,
                                      cpr::Timeout{static_cast<int>(opts_.timeout.count() * 1000)},
                                      cpr::SslOptions{
                                          .verify_host = ssl_options.verify_host,
                                          .verify_peer = ssl_options.verify_peer,
                                      });

    last_status = response.status_code;

    // Success
    if (response.status_code == 200) {
      hlog()->info("[ICS]: Fetched {} (attempt {}/{}), status={}, size={}",
                   url,
                   attempt + 1,
                   opts_.retries + 1,
                   response.status_code,
                   response.text.size());
      return response.text;
    }

    // Non-transient client error (4xx except 429) — don't retry
    if (response.status_code >= 400 && response.status_code < 500 && response.status_code != 429) {
      hlog()->error("[ICS]: Non-retryable HTTP error for {}: status={}", url, response.status_code);
      return {};
    }

    last_error = response.error.message;

    // Check if this is a transient error worth retrying
    if (!is_transient_error(last_error)) {
      hlog()->error("[ICS]: Non-transient error for {}: code={}, error={}", url, response.status_code, last_error);
      return {};
    }

    hlog()->warn("[ICS]: Transient error for {} (attempt {}/{}): status={}, error={}",
                 url,
                 attempt + 1,
                 opts_.retries + 1,
                 response.status_code,
                 last_error);
  }

  // All retries exhausted
  hlog()->error("[ICS]: Failed fetching {} after {} attempts: status={}, error={}",
                url,
                opts_.retries + 1,
                last_status,
                last_error);
  return {};
}

}  // namespace divoomdev::ics
