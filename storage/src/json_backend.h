#pragma once
#include <string>

#include "storage/i_storage.h"

namespace divoomdev::storage {

/// Реализация `i_settings_storage` поверх файла config.json.
///
/// Зеркалирует модель данных SQLite-схемы (accounts, calendar_sources,
/// schema_version). Все изменения сразу персистятся атомарной записью файла.
class json_backend final : public i_settings_storage {
 public:
  /// Открывает (или создаёт) файл по пути `path`. Бросает `std::runtime_error`
  /// при невозможности загрузить/создать файл.
  explicit json_backend(const std::string& path);
  ~json_backend() override = default;

  json_backend(const json_backend&) = delete;
  json_backend& operator=(const json_backend&) = delete;

  void upsert_account(const account& acc) override;
  std::optional<account> get_account(const std::string& service) const override;
  std::vector<account> list_accounts() const override;
  void delete_account(const std::string& service) override;

  void upsert_calendar_source(const calendar_source& src) override;
  std::vector<calendar_source> list_calendar_sources() const override;
  void remove_calendar_source(const std::string& url) override;

  void clear() override;

 private:
  void load();
  bool save() const;
  nlohmann::json& accounts_array() { return doc_["accounts"]; }
  nlohmann::json& sources_array() { return doc_["calendar_sources"]; }
  const nlohmann::json& accounts_array() const { return doc_["accounts"]; }
  const nlohmann::json& sources_array() const { return doc_["calendar_sources"]; }

  std::string path_;
  nlohmann::json doc_;
};

}  // namespace divoomdev::storage