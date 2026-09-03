#include "index.h"

namespace divoomdev::divoom::handlers::commands {

index::index(): handler(RequestType::POST) {
  request_.Command = "Channel/GetIndex";
}
index::~index() = default;

std::string index::get_path(const std::string_view host) const noexcept {
  return format("http://{}:9000/divoom_api", host);
}

bool index::handle(const std::string& data) {
  log()->info(data);
  return parse_json(data).get();
};

}  // namespace divoomdev::divoom::handlers::commands
