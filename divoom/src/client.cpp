#include "divoom/client.h"
#include "hare/defs.h"
#include <cpr/status_codes.h>

#include "divoom/handler.h"
#include "handlers/commands/brightness.h"
#include "handlers/commands/mirror.h"
#include "handlers/commands/reboot.h"
#include "handlers/commands/time_format.h"
#include "handlers/devices.h"

namespace divoomdev::divoom {
namespace {

cpr::Header get_base_headers() {
  return {
      //{"X-Yandex-Music-Client", user_id},
      //{"Authorization", format("OAuth {}", token)},
      {"Content-Type", "application/json"},
      {"accept", "application/json"},
  };
}

}  // namespace

client::client(std::string host): host_(std::move(host)) {
  AUTOTRACE;
}
client::~client() {
  AUTOTRACE;
}

std::vector<common::device> client::get_devices() {
  auto handler = std::make_shared<handlers::devices>();

  if (!execute<nullptr_t, std::vector<common::device>>(handler, host_)) {
    log()->error("");
    return {};
  }

  return handler->get_result();
}

bool client::do_reboot(const std::string& device_ip) {
  auto handler = std::make_shared<handlers::commands::reboot>();
  return execute<handlers::commands::command, nullptr_t>(handler, device_ip);
}

bool client::set_brightness(const std::string& device_ip, int brightness) {
  auto handler = std::make_shared<handlers::commands::brightness>(brightness);
  return execute<handlers::commands::command_brightness, nullptr_t>(handler, device_ip);
}

bool client::set_mirror(const std::string& device_ip, int mode) {
  auto handler = std::make_shared<handlers::commands::mirror>(mode);
  return execute<handlers::commands::command_mirror, nullptr_t>(handler, device_ip);
}

bool client::set_time_format(const std::string& device_ip, int mode) {
  auto handler = std::make_shared<handlers::commands::time_format>(mode);
  return execute<handlers::commands::command_timeformat, nullptr_t>(handler, device_ip);
}

template<typename T1, typename T2>
bool client::execute(const std::shared_ptr<handler<T1, T2>>& handler, std::string host) {
  assert(handler);

  try {
    AUTOMEASURE
    cpr::Url Url(handler->get_path(host));
    cpr::Parameters parameters;
    cpr::SslOptions ssl_options;
    ssl_options.verify_host = false;
    ssl_options.verify_peer = false;

    cpr::Response response;
    switch (handler->request_type) {
    case RequestType::GET:
      {
        response = cpr::Get(Url, parameters, get_base_headers(), ssl_options);
        break;
      }
    case RequestType::POST:
      {
        cpr::Body body(handler->get_request());
        response = cpr::Post(Url, body, parameters, get_base_headers(), ssl_options);
        break;
      }
    }

    if (response.error) {
      log()->error("CURL Error: {} {}", response.error.message, static_cast<int>(response.error.code));
      return false;
    }

    if (response.status_code != cpr::status::HTTP_OK) {
      log()->error("Code: {}", response.status_code);
      log()->error("Text: {}", response.text);
      return false;
    }

    if (!handler->handle(response.text)) {
      log()->error("Error handling data: {}", response.text);
      return false;
    }

  } catch (std::exception& ex) {
    log()->error(ex.what());
    return false;
  }

  return true;
}
}  // namespace divoomdev::divoom
