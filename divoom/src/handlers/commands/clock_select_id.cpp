#include "clock_select_id.h"

namespace divoomdev::divoom::handlers::commands {

clock_select_id::clock_select_id(int clock_id): handler(RequestType::POST) {
  request_.command = "Channel/SetClockSelectId";
  request_.ClockId = clock_id;
}
clock_select_id::~clock_select_id() = default;

std::string clock_select_id::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}
}  // namespace divoomdev::divoom::handlers::commands
