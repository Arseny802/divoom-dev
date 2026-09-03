#include "divoom/client.h"
#include "common/clock.h"
#include "hare/defs.h"
#include <cpr/status_codes.h>

#include "handlers/channel/brightness.h"
#include "handlers/channel/turn_screen.h"
#include "handlers/clock/device_clockes.h"
#include "handlers/clock/get_config.h"
#include "handlers/clock/select_id.h"
#include "handlers/clock/set_config.h"
#include "handlers/commands/all_conf.h"
#include "handlers/commands/index.h"
#include "handlers/commands/patch_clock_info.h"
#include "handlers/custom_control/enter.h"
#include "handlers/custom_control/exit.h"
#include "handlers/custom_control/update_text.h"
#include "handlers/device/clock_info.h"
#include "handlers/device/font_list.h"
#include "handlers/device/high_light.h"
#include "handlers/device/mirror.h"
#include "handlers/device/reboot.h"
#include "handlers/device/time_format.h"
#include "handlers/devices.h"
#include "handlers/login.h"

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

user_info client::login(std::string email, std::string password) {
  auto handler = std::make_shared<handlers::login>(std::move(email), std::move(password));

  if (!execute(handler, host_)) {
    log()->error("Login failed!");
    return {};
  }

  return handler->get_result();
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
  handlers::clock::get_list_request request;
  request.DeviceId = device_id;
  auto handler = std::make_shared<handlers::clock::get_list>(std::move(request));

  if (!execute(handler, host_)) {
    log()->error("Could not get device clock list!");
    return {};
  }

  return handler->get_result();
}
common::font_list client::get_font_list() {
  auto handler = std::make_shared<handlers::font_list>();

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

bool client::do_turn_screen(const std::string& device_ip, int turn_on) {
  auto handler = std::make_shared<handlers::commands::turn_screen>(turn_on);
  return execute(handler, device_ip);
}

bool client::get_all_conf(const std::string& device_ip) {
  auto handler = std::make_shared<handlers::commands::all_conf>();
  return execute(handler, device_ip);
}

clock_config client::get_clock_config(int device_id, int clock_id, const user_info& user) {
  auto handler = std::make_shared<handlers::clock::get_config>(device_id, clock_id, user);
  if (!execute(handler, host_)) {
    log()->error("Could not get clock config!");
    return {};
  }

  return handler->get_result();
}

bool client::get_clock_info(const std::string& device_ip) {
  auto handler = std::make_shared<handlers::commands::clock_info>();
  return execute(handler, device_ip);
}

bool client::get_index(const std::string& device_ip) {
  auto handler = std::make_shared<handlers::commands::index>();
  return execute(handler, device_ip);
}

bool client::set_brightness(const std::string& device_ip, int brightness) {
  auto handler = std::make_shared<handlers::commands::brightness>(brightness);
  return execute(handler, device_ip);
}

bool client::set_high_light(const std::string& device_ip, int mode) {
  auto handler = std::make_shared<handlers::commands::high_light>(mode);
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
  auto handler = std::make_shared<handlers::clock::select_id>(clock_id);
  return execute(handler, device_ip);
}

bool client::set_clock_config(int device_id, int clock_id, const user_info& user, const clock_config& clock_config) {
  auto handler = std::make_shared<handlers::clock::set_config>(device_id, clock_id, user, clock_config.ItemList);
  return execute(handler, host_);
}

bool client::patch_clock_info(const std::string& device_ip, common::display_rendered_list patch_list) {
  auto handler = std::make_shared<handlers::commands::patch_clock_info>(std::move(patch_list));
  return execute(handler, device_ip);
}

bool client::cusom_control_enter(const std::string& device_ip,
                                 common::display_rendered_list display_list,
                                 std::string backgroud_image_addr,
                                 int backgroud_image_local_flag) {
  auto handler = std::make_shared<handlers::custom_control::enter>(
      std::move(display_list), std::move(backgroud_image_addr), backgroud_image_local_flag);
  return execute(handler, device_ip);
}
bool client::cusom_control_exit(const std::string& device_ip) {
  auto handler = std::make_shared<handlers::custom_control::exit>();
  return execute(handler, device_ip);
}

bool client::cusom_control_update_text(const std::string& device_ip, common::display_list display_list) {
  auto handler = std::make_shared<handlers::custom_control::update_text>(std::move(display_list));
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

    return handler->handle(response.text);

  } catch (std::exception& ex) {
    log()->error(ex.what());
    return false;
  } catch (...) {
    return false;
  }
}
}  // namespace divoomdev::divoom
