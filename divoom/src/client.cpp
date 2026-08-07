#include "divoom/client.h"
#include "common/clock.h"
#include "hare/defs.h"
#include <cpr/status_codes.h>

#include "handlers/commands/brightness.h"
#include "handlers/commands/clock_select_id.h"
#include "handlers/commands/mirror.h"
#include "handlers/commands/reboot.h"
#include "handlers/commands/time_format.h"
#include "handlers/device_clockes.h"
#include "handlers/devices.h"

namespace divoomdev::divoom {
namespace {

cpr::Header get_base_headers() {
  return {
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

common::device_list client::get_devices() {
  auto handler = std::make_shared<handlers::devices>();

  if (!execute(handler, host_)) {
    log()->error("Could not get device list!");
    return {};
  }

  return handler->get_result();
}

common::clock_list client::get_device_clockes(int device_id) {
  handlers::device_clockes_request request;
  request.DeviceId = device_id;
  auto handler = std::make_shared<handlers::device_clockes>(std::move(request));

  if (!execute(handler, host_)) {
    log()->error("Could not get device clock list!");
    return {};
  }

  return handler->get_result();
}

bool client::do_reboot(const std::string& device_ip) {
  auto handler = std::make_shared<handlers::commands::reboot>();
  return execute(handler, device_ip);
}

bool client::set_brightness(const std::string& device_ip, int brightness) {
  auto handler = std::make_shared<handlers::commands::brightness>(brightness);
  return execute(handler, device_ip);
}

bool client::set_mirror(const std::string& device_ip, int mode) {
  auto handler = std::make_shared<handlers::commands::mirror>(mode);
  return execute(handler, device_ip);
}

bool client::set_time_format(const std::string& device_ip, int mode) {
  auto handler = std::make_shared<handlers::commands::time_format>(mode);
  return execute(handler, device_ip);
}
bool client::set_clock_id(const std::string& device_ip, int clock_id) {
  auto handler = std::make_shared<handlers::commands::clock_select_id>(clock_id);
  return execute(handler, device_ip);
}

template<typename HandlerT>
bool client::execute(const std::shared_ptr<HandlerT>& handler, std::string host) {
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
