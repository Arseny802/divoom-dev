#include "json_backend.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#if defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace divoomdev::storage {

namespace fs = std::filesystem;

namespace {

/// Атомарно заменяет `dest` содержимым `tmp` (перезапись существующего файла).
bool atomic_replace(const fs::path& tmp, const fs::path& dest) {
#if defined(_WIN32)
  if (::MoveFileExA(tmp.string().c_str(), dest.string().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    return true;
  }
#endif
  std::error_code ec;
  fs::remove(dest, ec);
  fs::rename(tmp, dest, ec);
  return !ec;
}

}  // namespace

json_backend::json_backend(const std::string& path): path_(path) {
  load();
}

void json_backend::load() {
  doc_ = nlohmann::json{
      {"schema_version", 1},
      {"accounts", nlohmann::json::array()},
      {"calendar_sources", nlohmann::json::array()},
  };

  std::ifstream in(path_, std::ios::binary);
  if (!in) {
    // Файл отсутствует — создаём дефолтный документ.
    if (!save()) {
      throw std::runtime_error("storage: cannot create json file '" + path_ + "'");
    }
    return;
  }

  try {
    nlohmann::json loaded = nlohmann::json::parse(in);
    doc_.update(loaded);  // восстанавливаем отсутствующие ключи значениями по умолчанию
  } catch (const nlohmann::json::exception& e) {
    throw std::runtime_error("storage: invalid json file '" + path_ + "': " + e.what());
  }
}

bool json_backend::save() const {
  const fs::path dest = fs::path(path_);
  const fs::path dir = dest.parent_path();
  if (!dir.empty()) {
    std::error_code ec;
    fs::create_directories(dir, ec);
  }

  const fs::path tmp = dest.string() + ".tmp";
  {
    std::ofstream out(tmp, std::ios::trunc);
    if (!out) {
      log()->error("storage: cannot open tmp file '{}' for write", tmp.string());
      return false;
    }
    out << doc_.dump(2);
  }

  if (!atomic_replace(tmp, dest)) {
    log()->error("storage: failed to atomically replace '{}'", path_);
    return false;
  }
  return true;
}

void json_backend::upsert_account(const account& acc) {
  auto& arr = accounts_array();
  for (auto& item: arr) {
    if (item.value("service", "") == acc.service) {
      item = acc;
      save();
      return;
    }
  }
  arr.push_back(acc);
  save();
}

std::optional<account> json_backend::get_account(const std::string& service) const {
  for (const auto& item: accounts_array()) {
    if (item.value("service", "") == service) {
      return item.get<account>();
    }
  }
  return std::nullopt;
}

std::vector<account> json_backend::list_accounts() const {
  std::vector<account> result;
  for (const auto& item: accounts_array()) {
    result.push_back(item.get<account>());
  }
  return result;
}

void json_backend::delete_account(const std::string& service) {
  auto& arr = accounts_array();
  for (auto it = arr.begin(); it != arr.end(); ++it) {
    if (it->value("service", "") == service) {
      arr.erase(it);
      save();
      return;
    }
  }
}

void json_backend::upsert_calendar_source(const calendar_source& src) {
  auto& arr = sources_array();
  for (auto& item: arr) {
    if (item.value("url", "") == src.url) {
      item = src;
      save();
      return;
    }
  }
  arr.push_back(src);
  save();
}

std::vector<calendar_source> json_backend::list_calendar_sources() const {
  std::vector<calendar_source> result;
  for (const auto& item: sources_array()) {
    result.push_back(item.get<calendar_source>());
  }
  std::sort(result.begin(), result.end(), [](const calendar_source& a, const calendar_source& b) {
    if (a.priority != b.priority) {
      return a.priority > b.priority;
    }
    return a.url < b.url;
  });
  return result;
}

void json_backend::remove_calendar_source(const std::string& url) {
  auto& arr = sources_array();
  for (auto it = arr.begin(); it != arr.end(); ++it) {
    if (it->value("url", "") == url) {
      arr.erase(it);
      save();
      return;
    }
  }
}

void json_backend::clear() {
  doc_["accounts"] = nlohmann::json::array();
  doc_["calendar_sources"] = nlohmann::json::array();
  save();
}

}  // namespace divoomdev::storage