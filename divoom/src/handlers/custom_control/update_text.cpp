#include "update_text.h"

namespace divoomdev::divoom::handlers::custom_control {

update_text::update_text(common::display_list display_list): handler(RequestType::POST) {
  request_.Command = "Device/UpdateDisplayItems";
  request_.DispList = std::move(display_list);
}
update_text::~update_text() = default;

std::string update_text::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool update_text::handle(const std::string& data) {
  return parse_json(data).get();
};
}  // namespace divoomdev::divoom::handlers::custom_control
