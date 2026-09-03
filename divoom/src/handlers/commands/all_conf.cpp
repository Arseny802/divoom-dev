#include "all_conf.h"

namespace divoomdev::divoom::handlers::commands {

all_conf::all_conf(): handler(RequestType::POST) {
  request_.Command = "Channel/GetConfig";
  request_.ChannelType = "Clock";
}
all_conf::~all_conf() = default;

std::string all_conf::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool all_conf::handle(const std::string& data) {
  log()->info(data);
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
