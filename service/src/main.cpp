#include "pch.h"
#include <boost/process/environment.hpp>
#include <chrono>
#include <ctime>
#include <exception>
#include <string>

#include "common/common.hpp"
#include "divoom/client.h"
#include "ics/event_manager.h"
#include "ics/http_source.h"
#include "storage/models.h"
#include "storage/storage.hpp"

namespace divoomdev::service {

/// Форматирует абсолютный момент времени в локальный часовой пояс (Europe/Moscow).
std::string fmt_local_time(std::chrono::time_point<std::chrono::system_clock> tp, const char* fmt) {
  auto t = std::chrono::system_clock::to_time_t(tp);
  struct tm tm;
  localtime_s(&tm, &t);
  char buf[64];
  strftime(buf, sizeof(buf), fmt, &tm);
  return std::string(buf);
}

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

void update_text(std::string text, const std::string& login, const std::string& password) {
  divoom::client client;

  const auto devices = client.get_devices();
  if (devices.empty()) {
    log()->warning("No devices found in LAN!!!");
    exit(1);
  }

  const auto user_info = client.login(login, password);
  log()->info("Token {}, User ID {}", user_info.Token, user_info.UserId);

  for (const auto& dev: devices) {
    log()->info("Deivice '{}'':"
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

int main() {
  std::unique_ptr<storage::i_settings_storage> storage =
      storage::open_storage(storage::backend_type::sqlite, "config.db");

  ics::calendar_settings settings;
  settings.horizon = std::chrono::hours{24};
  ics::event_manager manager;
  manager.set_settings(settings);

  for (auto item: storage->list_calendar_sources()) {
    manager.add_calendar_url(item.url);
  }

  std::string text;
  text.reserve(40 * sizeof(char*) * 512);

  for (const common::event& event: manager.get_next_48h_events()) {
    log()->info("{} {} {} {}",
                event.summary,
                event.location,
                fmt_local_time(event.start, "%Y-%m-%d %H:%M:%S"),
                fmt_local_time(event.end, "%Y-%m-%d %H:%M:%S"));
    text +=
        format("{}: {}-{}\n", event.summary, fmt_local_time(event.start, "%H:%M"), fmt_local_time(event.end, "%H:%M"));
  }

  auto all_accounts = storage->list_accounts();
  update_text(text, all_accounts[0].login, all_accounts[0].password);

  return EXIT_SUCCESS;
}
}  // namespace divoomdev::service

int main() {
  divoomdev::service::initialize_logging();
  return divoomdev::service::main();
}
