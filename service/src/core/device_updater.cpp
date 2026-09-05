#include "device_updater.h"
#include "divoom/client.h"

namespace divoomdev::service::core {

void device_updater::update(const std::string& text, const std::string& login, const std::string& password) {
  divoom::client client;

  const auto devices = client.get_devices();
  if (devices.empty()) {
    log()->warning("No devices found in LAN!!!");
    exit(1);
  }

  const auto user_info = client.login(login, password);
  log()->info("Token {}, User ID {}", user_info.Token, user_info.UserId);

  for (const auto& dev: devices) {
    log()->info("Device '{}'':"
                "\n\tID = {}\n\tip = {}\n\tmac = {}\n\thardware = {}\n",
                dev.name,
                dev.id,
                dev.private_ip,
                dev.mac,
                dev.hardware);

    auto clock_config = client.get_clock_config(dev.id, 598, user_info);

    for (auto& item: clock_config.ItemList) {
      if (item.ItemType == 25) {
        item.ItemValue = text;
      }
    }
    log()->info("set_clock_config: {}", client.set_clock_config(dev.id, 598, user_info, clock_config));
  }
}

}  // namespace divoomdev::service::core
