#include "turn_screen.h"

namespace divoomdev::divoom::handlers::commands {

turn_screen::turn_screen(int turn_on): handler(RequestType::POST) {
  request_.Command = "Channel/OnOffScreen";
  request_.OnOff = turn_on;
}
turn_screen::~turn_screen() = default;

std::string turn_screen::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool turn_screen::handle(const std::string& data) {
  return parse_json(data).get();
};
}  // namespace divoomdev::divoom::handlers::commands
