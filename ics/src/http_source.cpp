#include "ics/http_source.h"

#include "cpr/cpr.h"

namespace divoomdev::ics {

std::string http_calendar_source::fetch(const std::string& url) {
  cpr::Parameters parameters;
  cpr::SslOptions ssl_options;
  ssl_options.verify_host = false;
  ssl_options.verify_peer = false;
  cpr::Response response = cpr::Get(cpr::Url{url}, parameters, ssl_options);

  hlog()->info("[ICS]: Fetched {}, status={}, size={}", url, response.status_code, response.text.size());

  if (response.status_code != 200) {
    hlog()->error(
        "[ICS]: Failed fetching {} with code={}, error={}", url, response.status_code, response.error.message);
    return {};
  }

  return response.text;
}

}  // namespace divoomdev::ics