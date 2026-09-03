#include "select_id.h"

namespace divoomdev::divoom::handlers::clock {

select_id::select_id(int clock_id): handler(RequestType::POST) {
  request_.Command = "Channel/SetClockSelectId";
  request_.ClockId = clock_id;
}
select_id::~select_id() = default;

std::string select_id::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool select_id::handle(const std::string& data) {
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::clock
