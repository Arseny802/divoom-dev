#include "brightness.h"

namespace divoomdev::divoom::handlers::commands {

brightness::brightness(int brightness): handler(RequestType::POST) {
  request_.Command = "Channel/SetBrightness";
  request_.Brightness = brightness;
}
brightness::~brightness() = default;

std::string brightness::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool brightness::handle(const std::string& data) {
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
