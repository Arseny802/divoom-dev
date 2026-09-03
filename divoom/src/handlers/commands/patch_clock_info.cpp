#include "patch_clock_info.h"

namespace divoomdev::divoom::handlers::commands {

patch_clock_info::patch_clock_info(common::display_rendered_list patch_list): handler(RequestType::POST) {
  request_.Command = "Device/GetLocalClockInfo";
  request_.ItemPatchList = std::move(patch_list);
}
patch_clock_info::~patch_clock_info() = default;

std::string patch_clock_info::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool patch_clock_info::handle(const std::string& data) {
  log()->info(data);
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
