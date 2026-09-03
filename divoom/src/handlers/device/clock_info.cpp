#include "clock_info.h"

namespace divoomdev::divoom::handlers::commands {

clock_info::clock_info(): handler(RequestType::POST) {
  request_.Command = "Device/GetLocalClockInfo";
}
clock_info::~clock_info() = default;

std::string clock_info::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool clock_info::handle(const std::string& data) {
  log()->info(data);
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
