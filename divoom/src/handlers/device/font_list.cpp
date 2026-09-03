#include "font_list.h"

namespace divoomdev::divoom::handlers {
namespace {
common::font parse_device_object(const nlohmann::json& j) {
  common::font dev;
  dev.id = j.value("id", 0);
  dev.type = j.value("type", 0);
  dev.url = j.value("url", std::string(""));
  dev.char_set = j.value("charSet", std::string(""));
  dev.encryption = j.value("Encryption", std::string(""));
  return dev;
}
}  // namespace

font_list::font_list(): handler(RequestType::GET) {
  static_assert(std::is_same<decltype(request_), nullptr_t>::value);
}
font_list::~font_list() = default;

std::string font_list::get_path(const std::string_view host) const noexcept {
  return format("https://{}:443/Device/GetTimeDialFontV2", host);
}

bool font_list::handle(const std::string& json_str) {
  result_ = std::nullopt;
  auto json = parse_json(json_str);

  if (!json->contains("FontList") || !(*json)["FontList"].is_array()) {
    return false;
  }

  result_ = ResultType();
  for (const auto& device_json: (*json)["FontList"]) {
    result_->emplace_back(parse_device_object(device_json));
  }

  return true;
}
}  // namespace divoomdev::divoom::handlers
