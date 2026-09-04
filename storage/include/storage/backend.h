#pragma once
#include <string_view>

namespace divoomdev::storage {

/// Выбор бэкенда хранения настроек.
enum class backend_type {
  /// SQLite база данных (основной бэкенд, поддержка миграций).
  sqlite,
  /// Файл config.json — дублирует функционал БД для простых сценариев.
  json_file,
};

/// Строковое представление типа бэкенда (для логирования/диагностики).
constexpr std::string_view to_string(backend_type type) noexcept {
  switch (type) {
  case backend_type::sqlite: return "sqlite";
  case backend_type::json_file: return "json_file";
  }
  return "unknown";
}

}  // namespace divoomdev::storage