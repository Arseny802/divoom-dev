#include "storage/storage.hpp"

#include <memory>
#include <stdexcept>

#include "json_backend.h"
#include "sqlite_backend.h"

namespace divoomdev::storage {

hare::hlogger_ptr get_logger() {
  return log();
}

std::unique_ptr<i_settings_storage> open_storage(backend_type type, const std::string& path) {
  try {
    switch (type) {
    case backend_type::sqlite: return std::make_unique<sqlite_backend>(path);
    case backend_type::json_file: return std::make_unique<json_backend>(path);
    }
  } catch (const std::exception& e) {
    log()->error("storage: open failed ({}): {}", to_string(type), e.what());
    return nullptr;
  }
  return nullptr;
}

}  // namespace divoomdev::storage