#include "mirror.h"

namespace divoomdev::divoom::handlers::commands {

mirror::mirror(int mode): handler(RequestType::POST) {
  request_.command = "Device/SetMirrorMode";
  request_.mode = mode;
}
mirror::~mirror() = default;

std::string mirror::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}
}  // namespace divoomdev::divoom::handlers::commands
