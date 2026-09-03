#include "enter.h"

namespace divoomdev::divoom::handlers::custom_control {

enter::enter(common::display_rendered_list display_list,
             std::string backgroud_image_addr,
             int backgroud_image_local_flag)
    : handler(RequestType::POST) {
  request_.Command = "Device/EnterCustomControlMode";
  request_.DispList = std::move(display_list);
  request_.BackgroudImageAddr = std::move(backgroud_image_addr);
  request_.BackgroudImageLocalFlag = backgroud_image_local_flag;
  log()->info(nlohmann::json(request_).dump());
}
enter::~enter() = default;

std::string enter::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool enter::handle(const std::string& data) {
  log()->info(data);
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::custom_control
