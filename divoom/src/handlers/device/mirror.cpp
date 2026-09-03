#include "mirror.h"

namespace divoomdev::divoom::handlers::commands {

mirror::mirror(int mode): handler(RequestType::POST) {
  request_.Command = "Device/SetMirrorMode";
  request_.Mode = mode;
}
mirror::~mirror() = default;

std::string mirror::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool mirror::handle(const std::string& data) {
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
