#include "device_clockes.h"

namespace divoomdev::divoom::handlers {
namespace {
common::clock parse_device_object(const nlohmann::json& j) {
  common::clock clock;
  clock.id = j.value("ClockId", 0);
  clock.name = j.value("ClockName", std::string(""));
  clock.type = j.value("ClockType", 0);
  clock.image_pixel_id = j.value("ImagePixelId", std::string(""));
  clock.position = j.value("Position", 0);
  return clock;
}
}  // namespace

// C:\Users\a.ravin>curl -XPOST https://appin.divoom-gz.com/Channel/MyClockGetList -H "accept: application/json" -H
// "Content-Type: application/json" -d "{\"DeviceId\": 300417773, \"StartNum\": 1,\"EndNum\": 100, \"DeviceType\":
// \"Frame\"}"

device_clockes::device_clockes(device_clockes_request request): handler(RequestType::POST) {
  request_ = std::move(request);
}
device_clockes::~device_clockes() = default;

std::string device_clockes::get_path(const std::string_view host) const noexcept {
  return format("https://{}:443/Channel/MyClockGetList", host);
}

bool device_clockes::handle(const std::string& json_str) {
  result_ = std::nullopt;
  auto json = parse_json(json_str);

  if (!json->contains("ClockList") || !(*json)["ClockList"].is_array()) {
    return false;
  }

  result_ = ResultType();
  for (const auto& device_json: (*json)["ClockList"]) {
    result_->emplace_back(parse_device_object(device_json));
  }

  return true;
}
}  // namespace divoomdev::divoom::handlers
