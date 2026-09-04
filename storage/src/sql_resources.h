#pragma once
#include <string>
#include <vector>

namespace divoomdev::storage::detail {

/// Один SQL-скрипт миграции.
struct sql_script {
  /// Версия схемы, до которой доводит этот скрипт (соответствует финальному
  /// значению `PRAGMA user_version` внутри файла).
  int version = 0;
  /// Содержимое скрипта (встроено в бинарник через incbin).
  std::string sql;
};

/// Базовая схема (`sql/sqlite/db.sql`), применяется при первом открытии БД.
const std::string& baseline_schema();

/// Упорядоченный список инкрементальных миграций (по возрастанию версии).
const std::vector<sql_script>& migrations();

}  // namespace divoomdev::storage::detail