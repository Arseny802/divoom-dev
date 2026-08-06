#include "divoom/client_async.h"
#include "hare/defs.h"

namespace divoomdev::divoom {

client_async::client_async(std::string host): client(std::move(host)) {
  AUTOTRACE;
}
client_async::~client_async() {
  AUTOTRACE;
}

std::vector<common::device> client_async::get_devices() {
  std::vector<common::device> devices;

  return devices;
}

void client_async::add_request() { }
}  // namespace divoomdev::divoom
