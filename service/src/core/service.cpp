#include "service.h"
#include "device_updater.h"
#include "event_formatter.h"

#include "ics/event_manager.h"
#include "storage/i_storage.h"

namespace divoomdev::service::core {

service::service(storage_ptr storage)
    : storage_(std::move(storage)),
      device_updater_(std::make_unique<device_updater>()),
      event_formatter_(std::make_unique<event_formatter>()) { }

service::~service() = default;

void service::set_update_interval(std::chrono::minutes interval) {
  update_interval_ = interval;
}

void service::run() {
  log()->info("Service loop started");

  ics::calendar_settings settings;
  settings.horizon = std::chrono::hours{24};
  calendar_manager_ = std::make_unique<ics::event_manager>();
  calendar_manager_->set_settings(settings);

  for (auto item: storage_->list_calendar_sources()) {
    calendar_manager_->add_calendar_url(item.url);
  }

  while (true) {
    try {
      process_cycle();
    } catch (const std::exception& e) {
      log()->error("Error in service loop: {}", e.what());
    }

    std::this_thread::sleep_for(update_interval_);
  }
}

void service::process_cycle() {
  auto all_accounts = storage_->list_accounts();
  if (all_accounts.empty()) {
    log()->critical("No accounts found in storage");
    return;
  }

  auto events = calendar_manager_->get_next_events();
  std::string text = event_formatter_->format(events);
  device_updater_->update(text, all_accounts[0].login, all_accounts[0].password);
}

}  // namespace divoomdev::service::core
