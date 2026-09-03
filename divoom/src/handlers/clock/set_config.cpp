#include "set_config.h"

namespace divoomdev::divoom::handlers::clock {

set_config::set_config(int device_id, int clock_id, const user_info& user, clock_config::item_list_t item_list)
    : handler(RequestType::POST) {
  request_.Command = "Channel/SetClockConfig";
  request_.DeviceId = device_id;
  request_.ClockId = clock_id;
  request_.UserId = user.UserId;
  request_.Token = user.Token;
  request_.ItemList = std::move(item_list);
}
set_config::~set_config() = default;

std::string set_config::get_path(const std::string_view host) const noexcept {
  return format("https://{}:443/Channel/SetClockConfig", host);
}

bool set_config::handle(const std::string& json_str) {
  return parse_json(json_str).get();
};

}  // namespace divoomdev::divoom::handlers::clock
