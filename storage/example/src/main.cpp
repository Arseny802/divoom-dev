#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "storage/storage.hpp"

using divoomdev::storage::account;
using divoomdev::storage::backend_type;
using divoomdev::storage::calendar_source;
using divoomdev::storage::i_settings_storage;
using divoomdev::storage::open_storage;

namespace {

void demo_accounts(i_settings_storage& store) {
  store.upsert_account(account{"divoom", "user@example.com", "s3cret"});

  if (const auto acc = store.get_account("divoom"); acc) {
    std::cout << "account: service=" << acc->service << " login=" << acc->login << " password=" << acc->password
              << "\n";
  }

  store.upsert_account(account{"divoom", "new@example.com", "new-pass"});
  std::cout << "accounts count after update: " << store.list_accounts().size() << "\n";

  store.delete_account("divoom");
  std::cout << "account present after delete: " << (store.get_account("divoom").has_value() ? "yes" : "no") << "\n";
}

void demo_sources(i_settings_storage& store) {
  store.upsert_calendar_source(calendar_source{"https://cal.example/primary.ics", 10, true});
  store.upsert_calendar_source(calendar_source{"https://cal.example/work.ics", 5, true});
  store.upsert_calendar_source(calendar_source{"https://cal.example/personal.ics", 1, false});

  std::cout << "calendar sources (by priority desc):\n";
  for (const auto& s: store.list_calendar_sources()) {
    std::cout << "  url=" << s.url << " priority=" << s.priority << " enabled=" << (s.enabled ? "true" : "false")
              << "\n";
  }

  store.remove_calendar_source("https://cal.example/work.ics");
  std::cout << "sources count after remove: " << store.list_calendar_sources().size() << "\n";
}

void demo(backend_type type, const std::string& path) {
  std::cout << "=== backend: " << divoomdev::storage::to_string(type) << " path=" << path << " ===\n";

  auto store = open_storage(type, path);
  if (!store) {
    std::cout << "failed to open storage\n";
    return;
  }

  demo_accounts(*store);
  demo_sources(*store);
  store->clear();
  std::cout << "cleared, remaining accounts=" << store->list_accounts().size()
            << " sources=" << store->list_calendar_sources().size() << "\n";
}

}  // namespace

int main() {
  demo(backend_type::sqlite, "storage_example.db");
  demo(backend_type::json_file, "config.json");
  return EXIT_SUCCESS;
}