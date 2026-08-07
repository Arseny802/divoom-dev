#include "devices.h"

namespace divoomdev::divoom::handlers {
namespace {
common::device parse_device_object(const nlohmann::json& j) {
  common::device dev;

  // Преобразование camelCase ключей JSON в поля структуры
  dev.name = j.value("DeviceName", std::string(""));
  dev.id = j.value("DeviceId", 0);
  dev.private_ip = j.value("DevicePrivateIP", std::string(""));
  dev.mac = j.value("DeviceMac", std::string(""));
  dev.hardware = j.value("Hardware", 0);

  return dev;
}
}  // namespace

devices::devices(): handler(RequestType::GET) {
  static_assert(std::is_same<decltype(request_), nullptr_t>::value);
}
devices::~devices() = default;

std::string devices::get_path(const std::string_view host) const noexcept {
  return format("https://{}:443/Device/ReturnSameLANDevice", host);
}

bool devices::handle(const std::string& json_str) {
  result_ = std::nullopt;
  auto json = parse_json(json_str);

  if (!json->contains("DeviceList") || !(*json)["DeviceList"].is_array()) {
    return false;
  }

  result_ = ResultType();
  for (const auto& device_json: (*json)["DeviceList"]) {
    result_->emplace_back(parse_device_object(device_json));
  }

  return true;
}
}  // namespace divoomdev::divoom::handlers
