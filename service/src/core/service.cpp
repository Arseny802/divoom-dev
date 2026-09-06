#include "service.h"
#include "device_updater.h"
#include "event_formatter.h"
#include "jenkins/build_info.h"
#include "jira/issue.h"
#include "lifecycle/service_state.h"

#include "ics/event_manager.h"
#include "jenkins/client.h"
#include "jira/client.h"
#include "storage/i_storage.h"

namespace divoomdev::service::core {

service::service(storage_ptr storage)
    : storage_(std::move(storage)),
      device_updater_(std::make_unique<device_updater>()),
      event_formatter_(std::make_unique<event_formatter>()) {
  AUTOTRACEF;
}

service::service(storage_ptr storage,
                 std::unique_ptr<device_updater> updater,
                 std::unique_ptr<event_formatter> formatter,
                 std::unique_ptr<ics::event_manager> calendar)
    : storage_(std::move(storage)),
      device_updater_(std::move(updater)),
      event_formatter_(std::move(formatter)),
      calendar_manager_(std::move(calendar)) {
  AUTOTRACEF;
}

service::~service() {
  AUTOTRACEF;
}

service::service(service&& other)
    : storage_(std::move(other.storage_)),
      device_updater_(std::move(other.device_updater_)),
      event_formatter_(std::move(other.event_formatter_)),
      update_interval_(other.update_interval_) {
  AUTOTRACEF;
}

service& service::operator=(service&& other) {
  AUTOTRACEF;
  storage_ = std::move(other.storage_);
  device_updater_ = std::move(other.device_updater_);
  event_formatter_ = std::move(other.event_formatter_);
  update_interval_ = other.update_interval_;
  return *this;
}

void service::set_update_interval(std::chrono::minutes interval) {
  AUTOTRACEF;
  update_interval_ = interval;  // TODO: make configurable update_interval
}

storage::i_settings_storage* service::get_storage() {
  AUTOTRACEF;
  if (!storage_) {
    log()->warning("Storage not initialized!");
    return nullptr;
  }
  return storage_.get();
}

void service::run() {
  log()->info("Service loop started");

  ics::calendar_settings settings;
  settings.horizon = std::chrono::hours{24};  // TODO: make configurable horizon
  calendar_manager_ = std::make_unique<ics::event_manager>();
  calendar_manager_->set_settings(settings);

  while (lifecycle::g_service_running() && !lifecycle::g_service_paused()) {
    try {
      get_db_info();
      process_cycle();
    } catch (const std::exception& e) {
      log()->error("Error in service loop: {}", e.what());
    } catch (...) {
      log()->critical("Unknown error in service loop!");
    }

    // Ждём с проверкой состояния паузы и остановки (каждые 100 мс)
    const unsigned long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(update_interval_).count();
    for (auto i = 0ull; i < total_ms / 100 && lifecycle::g_service_running() && !lifecycle::g_service_paused(); ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  if (lifecycle::g_service_paused()) {
    log()->info("Service loop paused");
  } else {
    log()->info("Service loop stopped");
  }
}

void service::pause() {
  AUTOTRACEF;
  lifecycle::g_service_paused() = true;
}

void service::resume() {
  AUTOTRACEF;
  lifecycle::g_service_paused() = false;
}

void service::stop() {
  AUTOTRACEF;
  lifecycle::g_service_running() = false;
}

void service::process_cycle() {
  AUTOFLUSH;
  AUTOMEASUREF;
  auto events = calendar_manager_->get_next_events();
  std::string text = event_formatter_->format(events);

  const auto jira_account = storage_->get_account("jira");
  const auto jira_url = storage_->get_meta("jira_url");
  if (jira_account.has_value() && jira_url.has_value()) {
    jira::client::config cfg;
    cfg.base_url = jira_url.value();
    cfg.email = jira_account->login;
    cfg.api_token = jira_account->password;
    jira::client client(cfg);

    jira::issues_vec result = client.GetActiveUserIssues();
    if (!result.empty()) {
      for (auto& item: result) {
        text += item.key + ": " + item.summary + '\n';
      }
    } else {
      text += " - Нет задач в Jira\n";
    }
  } else {
    log()->warning("Jira account or URL not found in storage!");
  }

  const auto jenkins_account = storage_->get_account("jenkins");
  const auto jenkins_url = storage_->get_meta("jenkins_url");
  if (jenkins_account.has_value() && jenkins_url.has_value()) {
    jenkins::client::config cfg;
    cfg.base_url = jenkins_url.value();
    cfg.username = jenkins_account->login;
    cfg.api_token = jenkins_account->password;
    jenkins::client client(cfg);

    jenkins::build_info_list result;
    constexpr std::array jobs = {"build/job/custom/job/windows/job/git",
                                 "build/job/custom/job/debian/job/git",
                                 "build/job/custom/job/rockchip/job/git",
                                 "build/job/custom/job/lnvr/job/git"};
    std::for_each(jobs.begin(), jobs.end(), [&client, &cfg, &result](const auto& job) {
      auto builds = client.GetUserBuilds(job, cfg.username, 100);
      result.insert(result.end(), builds.begin(), builds.end());
    });

    if (!result.empty()) {
      for (auto& item: result) {
        text += std::to_string(item.number) + ": " + item.result + " (" + item.status + ")\n";
      }
    } else {
      text += " - Нет сборок в Jenkins\n";
    }
  } else {
    log()->warning("Jenkins account or URL not found in storage!");
  }

  device_updater_->update(text);
}

void service::get_db_info() {
  AUTOTRACEF;
  if (!storage_) {
    log()->fatal("Storage not initialized!");
    return;
  }

  for (auto item: storage_->list_calendar_sources()) {
    calendar_manager_->add_calendar_url(item.url);
  }

  const auto divoom_account = storage_->get_account("divoom");
  if (divoom_account.has_value()) {
    device_updater_->update_credentials(divoom_account->login, divoom_account->password);
  } else {
    log()->warning("Divoom account not found in storage!");
  }
}

}  // namespace divoomdev::service::core
