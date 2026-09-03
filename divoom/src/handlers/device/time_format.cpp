#include "time_format.h"

namespace divoomdev::divoom::handlers::commands {

time_format::time_format(int mode): handler(RequestType::POST) {
  request_.Command = "Device/SetTime24Flag";
  request_.Mode = mode;
}
time_format::~time_format() = default;

std::string time_format::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool time_format::handle(const std::string& data) {
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
