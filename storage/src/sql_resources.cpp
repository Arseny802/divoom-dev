#include "sql_resources.h"

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#ifndef STORAGE_SQL_DIR
#  error "STORAGE_SQL_DIR compile definition is required (set in storage/CMakeLists.txt)"
#endif

// На GCC/Clang incbin эмбедит файл прямо в секцию .rodata бинарника.
// На MSVC макрос INCBIN лишь объявляет extern-символы (данные не встраиваются),
// поэтому для MSVC используется fallback — чтение файла с диска из
// STORAGE_SQL_DIR во время выполнения.
#if !defined(_MSC_VER)
#  include <incbin.h>
INCBIN(sql_baseline, STORAGE_SQL_DIR "/sql/sqlite/db.sql");

// Пример добавления новой миграции (см. sql/sqlite/migrations/README.md):
// INCBIN(sql_migration_0002, STORAGE_SQL_DIR "/sql/sqlite/migrations/0002_example.sql");
#endif

namespace divoomdev::storage::detail {
namespace {

std::string read_file(const char* path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return {};
  }
  return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::string load_baseline() {
#if defined(_MSC_VER)
  return read_file(STORAGE_SQL_DIR "/sql/sqlite/db.sql");
#else
  return std::string(reinterpret_cast<const char*>(gsql_baselineData), gsql_baselineSize);
#endif
}

}  // namespace

const std::string& baseline_schema() {
  static const std::string sql = load_baseline();
  return sql;
}

const std::vector<sql_script>& migrations() {
  static const std::vector<sql_script> list = {
      // Пример регистрации миграции:
      // {2, load_migration<gsql_migration_0002>("0002_example")},
  };
  return list;
}

}  // namespace divoomdev::storage::detail