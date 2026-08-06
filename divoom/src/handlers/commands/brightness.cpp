#include "brightness.h"

namespace divoomdev::divoom::handlers::commands {

brightness::brightness(int brightness): handler(RequestType::POST) {
  request_.command = "Channel/SetBrightness";
  request_.brightness = brightness;
}
brightness::~brightness() = default;

std::string brightness::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}
}  // namespace divoomdev::divoom::handlers::commands
