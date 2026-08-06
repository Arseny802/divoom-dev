#include "pch.h"
#include <boost/process/environment.hpp>
#include <exception>

#include "common/common.hpp"
#include "divoom/client.h"
#include "storage/storage.hpp"

namespace divoomdev::service {

void initialize_logging() {
  log()->info("Application started. PID: {}", boost::this_process::get_id());
  std::ignore = storage::get_logger();
}

int main() {
  divoom::client client;

  const auto devices = client.get_devices();
  for (const auto& dev: devices) {
    std::cout << "Name: " << dev.name << ", ID: " << dev.id << ", IP: " << dev.private_ip << ", MAC: " << dev.mac
              << ", HW: " << dev.hardware << std::endl;

    log()->info("set_brightness: {}", client.set_brightness(dev.private_ip, 20));
    // log()->info("set_mirror: {}", client.set_mirror(dev.private_ip, 0));
    // log()->info("set_time_format: {}", client.set_time_format(dev.private_ip, 1));
    // log()->info("do_reboot: {}", client.do_reboot(dev.private_ip));
  }

  return EXIT_SUCCESS;
}
}  // namespace divoomdev::service

int main() {
  divoomdev::service::initialize_logging();
  return divoomdev::service::main();
}
