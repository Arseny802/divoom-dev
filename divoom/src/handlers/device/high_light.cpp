#include "high_light.h"

namespace divoomdev::divoom::handlers::commands {

high_light::high_light(int mode): handler(RequestType::POST) {
  request_.Command = "Device/SetHighLightMode";
  request_.Mode = mode;
}
high_light::~high_light() = default;

std::string high_light::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool high_light::handle(const std::string& data) {
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
