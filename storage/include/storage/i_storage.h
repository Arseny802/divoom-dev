#pragma once
#include <optional>
#include <string>
#include <vector>

#include "storage/models.h"

namespace divoomdev::storage {

/// Абстрактный интерфейс хранилища настроек.
///
/// Обе реализации (SQLite и JSON-файл) предоставляют один и тот же набор
/// операций, поэтому потребитель может переключать бэкенд простой передачей
/// аргумента в `open_storage()` без изменения кода.
class i_settings_storage {
 public:
  virtual ~i_settings_storage() = default;

  // ---- Учётные записи ------------------------------------------------------

  /// Создаёт или обновляет учётную запись (upsert по `service`).
  virtual void upsert_account(const account& acc) = 0;
  /// Возвращает учётную запись по имени сервиса, либо `nullopt`.
  virtual std::optional<account> get_account(const std::string& service) const = 0;
  /// Возвращает все учётные записи.
  virtual std::vector<account> list_accounts() const = 0;
  /// Удаляет учётную запись. Не ошибка, если её нет.
  virtual void delete_account(const std::string& service) = 0;

  // ---- ICS-источники -------------------------------------------------------

  /// Создаёт или обновляет источник (upsert по `url`).
  virtual void upsert_calendar_source(const calendar_source& src) = 0;
  /// Возвращает все источники, отсортированные по приоритету (по убыванию).
  virtual std::vector<calendar_source> list_calendar_sources() const = 0;
  /// Удаляет источник по URL. Не ошибка, если его нет.
  virtual void remove_calendar_source(const std::string& url) = 0;

  // ---- Метаданные (ключ-значение) ------------------------------------------

  /// Сохраняет значение по ключу. Если ключ уже существует — перезаписывает.
  virtual void set_meta(const std::string& key, const std::string& value) = 0;
  /// Возвращает значение по ключу, либо `nullopt` если ключ отсутствует.
  virtual std::optional<std::string> get_meta(const std::string& key) const = 0;
  /// Возвращает все пары ключ-значение.
  virtual std::vector<std::pair<std::string, std::string>> list_meta() const = 0;
  /// Удаляет ключ. Не ошибка, если его нет.
  virtual void delete_meta(const std::string& key) = 0;

  // ---- Обслуживание --------------------------------------------------------

  /// Полностью очищает хранилище (все таблицы/ключи).
  virtual void clear() = 0;
};

}  // namespace divoomdev::storage