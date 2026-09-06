#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "common/device.h"
#include "divoom/client.h"
#include "divoom/user_info.h"
#include "ics/event_manager.h"
#include "storage/i_storage.h"

namespace service_test {

// ============================================================================
// Mock: i_settings_storage
// ============================================================================
class mock_settings_storage final : public divoomdev::storage::i_settings_storage {
 public:
  std::vector<divoomdev::storage::account> list_accounts() const override { return accounts_; }

  std::optional<divoomdev::storage::account> get_account(const std::string& name) const override {
    for (const auto& a: accounts_) {
      if (a.service == name)
        return a;
    }
    return std::nullopt;
  }

  void upsert_account(const divoomdev::storage::account& acc) override {
    for (auto& a: accounts_) {
      if (a.service == acc.service) {
        a = acc;
        return;
      }
    }
    accounts_.push_back(acc);
  }

  void delete_account(const std::string& name) override {
    accounts_.erase(std::remove_if(accounts_.begin(),
                                   accounts_.end(),
                                   [&name](const divoomdev::storage::account& a) { return a.service == name; }),
                    accounts_.end());
  }

  std::vector<divoomdev::storage::calendar_source> list_calendar_sources() const override { return sources_; }

  void upsert_calendar_source(const divoomdev::storage::calendar_source& src) override {
    for (auto& s: sources_) {
      if (s.url == src.url) {
        s = src;
        return;
      }
    }
    sources_.push_back(src);
  }

  void remove_calendar_source(const std::string& url) override {
    sources_.erase(std::remove_if(sources_.begin(),
                                  sources_.end(),
                                  [&url](const divoomdev::storage::calendar_source& s) { return s.url == url; }),
                   sources_.end());
  }

  void clear() override {
    accounts_.clear();
    sources_.clear();
  }

  std::vector<divoomdev::storage::account> accounts_;
  std::vector<divoomdev::storage::calendar_source> sources_;
};

// ============================================================================
// Mock: divoom::client
// ============================================================================
class mock_divoom_client final : public divoomdev::divoom::client {
 public:
  using client::client;

  // Override iclient::get_devices
  std::vector<divoomdev::common::device> get_devices() override { return devices_; }

  // Configurable responses
  std::vector<divoomdev::common::device> devices_;
  divoomdev::divoom::user_info mock_user_info_{.Token = 12345, .UserId = 1};
  divoomdev::divoom::clock_config mock_clock_config_;
  bool set_clock_config_result_ = true;

  // Track calls
  int get_devices_call_count = 0;
};

// ============================================================================
// Mock: ics::event_manager
// ============================================================================
class mock_event_manager final : public divoomdev::ics::event_manager {
 public:
  using event_manager::event_manager;

  // Override get_next_events to return mock data
  divoomdev::ics::scheduled_event_list get_next_events() override { return mock_events_; }

  // Override get_next_events_scheduled
  divoomdev::ics::scheduled_event_list get_next_events_scheduled() override {
    return event_manager::get_next_events_scheduled();
  }

  std::vector<divoomdev::ics::scheduled_event> mock_events_;
  int get_next_events_call_count = 0;
};

}  // namespace service_test
