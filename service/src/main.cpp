#include "pch.h"
#include <boost/process/environment.hpp>
#include <exception>
#include <string>

#include "common/common.hpp"
#include "divoom/client.h"
#include "storage/storage.hpp"

namespace divoomdev::service {

void initialize_logging() {
  log()->info("Application started. PID: {}", boost::this_process::get_id());
  std::ignore = storage::get_logger();
}

void print_clocked(common::device_list devices, divoom::client client) {
  for (const auto& dev: devices) {
    log()->info("Deivice '{}'':"
                "\n\tID = {}\n\tip = {}\n\tmac = {}\n\thardware = {}\n",
                dev.name,
                dev.id,
                dev.private_ip,
                dev.mac,
                dev.hardware);

    const auto clockes = client.get_device_clockes(dev.id);
    for (const auto& clock: clockes) {
      log()->info("Deivice {} has clock:"
                  "\n\tID = {}\n\tname = {}\n\ttype = {}\n\tposition = {}\n\timage_pixel_id = {}\n",
                  dev.id,
                  clock.id,
                  clock.name,
                  clock.type,
                  clock.position,
                  clock.image_pixel_id);
    }
  }
}

int main() {
  divoom::client client;

  const auto devices = client.get_devices();
  if (devices.empty()) {
    log()->warning("No devices found in LAN!!!");
    return EXIT_SUCCESS;
  }

  log()->info("Token {}, User ID {}", user_info.Token, user_info.UserId);

  for (const auto& dev: devices) {
    log()->info("Deivice '{}'':"
                "\n\tID = {}\n\tip = {}\n\tmac = {}\n\thardware = {}\n",
                dev.name,
                dev.id,
                dev.private_ip,
                dev.mac,
                dev.hardware);

    // print_clocked(devices, client);

    // log()->info("set_brightness: {}", client.set_brightness(dev.private_ip, 20));
    //  log()->info("set_mirror: {}", client.set_mirror(dev.private_ip, 0));
    //  log()->info("set_time_format: {}", client.set_time_format(dev.private_ip, 1));
    // log()->info("do_reboot: {}", client.do_reboot(dev.private_ip));
    // log()->info("get_all_conf: {}", client.get_all_conf(dev.private_ip));
    // log()->info("get_clock_info: {}", client.get_clock_info(common::DIVOOMDEV_HOST));
    // log()->info("get_clock_info: {}", client.get_clock_info(dev.private_ip));
    //   log()->info("set_clock_id: {}", client.set_clock_id(dev.private_ip, 383897));
    //   log()->info("set_clock_id: {}", client.set_clock_id(dev.private_ip, 383923));

    auto clock_config = client.get_clock_config(dev.id, 598, user_info);

    for (auto& item: clock_config.ItemList) {
      if (item.ItemType == 25) {
        item.ItemValue = "Привет, мир!";
      }
    }
    log()->info("set_clock_config: {}", client.set_clock_config(dev.id, 598, user_info, clock_config));
  }

  return EXIT_SUCCESS;
}
}  // namespace divoomdev::service

int main() {
  divoomdev::service::initialize_logging();
  return divoomdev::service::main();
}
