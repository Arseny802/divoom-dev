#include "reboot.h"

namespace divoomdev::divoom::handlers::commands {

reboot::reboot(): handler(RequestType::POST) {
  request_.Command = "Device/SysReboot";
}
reboot::~reboot() = default;

std::string reboot::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool reboot::handle(const std::string& data) {
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
