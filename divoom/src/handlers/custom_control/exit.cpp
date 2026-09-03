#include "exit.h"

namespace divoomdev::divoom::handlers::custom_control {

exit::exit(): handler(RequestType::POST) {
  request_.Command = "Device/ExitCustomControlMode";
}
exit::~exit() = default;

std::string exit::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

}  // namespace divoomdev::divoom::handlers::custom_control
