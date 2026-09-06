#pragma once
#include <string>

#include "storage/i_storage.h"

struct sqlite3;

namespace divoomdev::storage {

/// Реализация `i_settings_storage` поверх SQLite.
///
/// При первом открытии пустой БД применяет базовую схему и миграции, скрипты
/// которых встроены в бинарник через incbin (см. sql_resources).
class sqlite_backend final : public i_settings_storage {
 public:
  /// Открывает (или создаёт) БД по пути `path` и применяет миграции.
  /// Бросает `std::runtime_error` при невозможности открыть/мигрировать.
  explicit sqlite_backend(const std::string& path);
  ~sqlite_backend() override;

  sqlite_backend(const sqlite_backend&) = delete;
  sqlite_backend& operator=(const sqlite_backend&) = delete;

  void upsert_account(const account& acc) override;
  std::optional<account> get_account(const std::string& service) const override;
  std::vector<account> list_accounts() const override;
  void delete_account(const std::string& service) override;

  void upsert_calendar_source(const calendar_source& src) override;
  std::vector<calendar_source> list_calendar_sources() const override;
  void remove_calendar_source(const std::string& url) override;

  void set_meta(const std::string& key, const std::string& value) override;
  std::optional<std::string> get_meta(const std::string& key) const override;
  std::vector<std::pair<std::string, std::string>> list_meta() const override;
  void delete_meta(const std::string& key) override;

  void clear() override;

 private:
  void open(const std::string& path);
  void apply_migrations();
  int current_version() const;
  bool exec(const char* sql, std::string& err);
  bool run_in_transaction(const std::string& sql);

  sqlite3* db_ = nullptr;
};

}  // namespace divoomdev::storage