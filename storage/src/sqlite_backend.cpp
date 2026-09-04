#include "sqlite_backend.h"

#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "sql_resources.h"

namespace divoomdev::storage {

namespace {

/// RAII-обёртка над подготовленным SQLite-выражением.
class sqlite_stmt {
 public:
  sqlite_stmt(sqlite3* db, const char* sql) {
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr) != SQLITE_OK) {
      log()->error("storage: failed to prepare '{}': {}", sql, sqlite3_errmsg(db));
      stmt_ = nullptr;
    }
  }
  ~sqlite_stmt() {
    if (stmt_ != nullptr) {
      sqlite3_finalize(stmt_);
    }
  }

  sqlite_stmt(const sqlite_stmt&) = delete;
  sqlite_stmt& operator=(const sqlite_stmt&) = delete;

  explicit operator bool() const { return stmt_ != nullptr; }
  sqlite3_stmt* get() const { return stmt_; }

  void bind_text(int idx, const std::string& value) {
    sqlite3_bind_text(stmt_, idx, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
  }
  void bind_int(int idx, int value) { sqlite3_bind_int(stmt_, idx, value); }

 private:
  sqlite3_stmt* stmt_ = nullptr;
};

/// Выполняет `step` и возвращает true, если строка получена (SQLITE_ROW).
bool step_row(sqlite3_stmt* st) {
  return sqlite3_step(st) == SQLITE_ROW;
}

}  // namespace

sqlite_backend::sqlite_backend(const std::string& path) {
  open(path);
}

sqlite_backend::~sqlite_backend() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
  }
}

void sqlite_backend::open(const std::string& path) {
  const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
  if (sqlite3_open_v2(path.c_str(), &db_, flags, nullptr) != SQLITE_OK) {
    const char* msg = db_ != nullptr ? sqlite3_errmsg(db_) : "unknown error";
    const std::string err = std::string("storage: cannot open sqlite database '") + path + "': " + msg;
    if (db_ != nullptr) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
    throw std::runtime_error(err);
  }

  std::string err;
  if (!exec("PRAGMA journal_mode=WAL;", err)) {
    log()->warn("storage: WAL mode unavailable: {}", err);
  }
  if (!exec("PRAGMA foreign_keys=ON;", err)) {
    log()->warn("storage: foreign_keys pragma failed: {}", err);
  }
  if (!exec("PRAGMA busy_timeout=5000;", err)) {
    log()->warn("storage: busy_timeout pragma failed: {}", err);
  }

  apply_migrations();
}

int sqlite_backend::current_version() const {
  sqlite_stmt st(db_, "PRAGMA user_version;");
  int version = 0;
  if (st && step_row(st.get())) {
    version = sqlite3_column_int(st.get(), 0);
  }
  return version;
}

bool sqlite_backend::exec(const char* sql, std::string& err) {
  char* zErr = nullptr;
  const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &zErr);
  if (rc != SQLITE_OK) {
    err = zErr != nullptr ? zErr : sqlite3_errmsg(db_);
    sqlite3_free(zErr);
    return false;
  }
  return true;
}

bool sqlite_backend::run_in_transaction(const std::string& sql) {
  std::string err;
  if (!exec("BEGIN;", err)) {
    log()->error("storage: BEGIN failed: {}", err);
    return false;
  }
  if (!exec(sql.c_str(), err)) {
    log()->error("storage: script failed: {}", err);
    exec("ROLLBACK;", err);
    return false;
  }
  if (!exec("COMMIT;", err)) {
    log()->error("storage: COMMIT failed: {}", err);
    exec("ROLLBACK;", err);
    return false;
  }
  return true;
}

void sqlite_backend::apply_migrations() {
  int version = current_version();

  if (version == 0) {
    const std::string& base = detail::baseline_schema();
    if (base.empty()) {
      log()->error("storage: baseline schema is empty, cannot initialise database");
      throw std::runtime_error("storage: baseline schema is empty");
    }
    if (!run_in_transaction(base)) {
      throw std::runtime_error("storage: failed to apply baseline schema");
    }
    version = current_version();
    log()->info("storage: baseline schema applied, version={}", version);
  }

  for (const auto& migration: detail::migrations()) {
    if (migration.version <= version) {
      continue;
    }
    if (migration.sql.empty()) {
      log()->warn("storage: migration to v{} is empty, skipped", migration.version);
      continue;
    }
    if (run_in_transaction(migration.sql)) {
      version = migration.version;
      log()->info("storage: migration applied, version={}", version);
    } else {
      log()->error("storage: migration to v{} FAILED", migration.version);
      throw std::runtime_error("storage: failed to apply migration to version " + std::to_string(migration.version));
    }
  }
}

void sqlite_backend::upsert_account(const account& acc) {
  sqlite_stmt st(db_,
                 "INSERT INTO accounts(service, login, password) VALUES(?1,?2,?3) "
                 "ON CONFLICT(service) DO UPDATE SET "
                 "login=excluded.login, password=excluded.password, "
                 "updated_at=strftime('%Y-%m-%dT%H:%M:%SZ','now');");
  if (!st) {
    return;
  }
  st.bind_text(1, acc.service);
  st.bind_text(2, acc.login);
  st.bind_text(3, acc.password);
  sqlite3_step(st.get());
}

std::optional<account> sqlite_backend::get_account(const std::string& service) const {
  sqlite_stmt st(db_, "SELECT service, login, password FROM accounts WHERE service=?1;");
  if (!st) {
    return std::nullopt;
  }
  st.bind_text(1, service);
  if (!step_row(st.get())) {
    return std::nullopt;
  }
  account acc;
  acc.service = reinterpret_cast<const char*>(sqlite3_column_text(st.get(), 0));
  acc.login = reinterpret_cast<const char*>(sqlite3_column_text(st.get(), 1));
  acc.password = reinterpret_cast<const char*>(sqlite3_column_text(st.get(), 2));
  return acc;
}

std::vector<account> sqlite_backend::list_accounts() const {
  std::vector<account> result;
  sqlite_stmt st(db_, "SELECT service, login, password FROM accounts ORDER BY service;");
  if (!st) {
    return result;
  }
  while (step_row(st.get())) {
    account acc;
    acc.service = reinterpret_cast<const char*>(sqlite3_column_text(st.get(), 0));
    acc.login = reinterpret_cast<const char*>(sqlite3_column_text(st.get(), 1));
    acc.password = reinterpret_cast<const char*>(sqlite3_column_text(st.get(), 2));
    result.push_back(std::move(acc));
  }
  return result;
}

void sqlite_backend::delete_account(const std::string& service) {
  sqlite_stmt st(db_, "DELETE FROM accounts WHERE service=?1;");
  if (!st) {
    return;
  }
  st.bind_text(1, service);
  sqlite3_step(st.get());
}

void sqlite_backend::upsert_calendar_source(const calendar_source& src) {
  sqlite_stmt st(db_,
                 "INSERT INTO calendar_sources(url, priority, enabled) VALUES(?1,?2,?3) "
                 "ON CONFLICT(url) DO UPDATE SET priority=excluded.priority, enabled=excluded.enabled;");
  if (!st) {
    return;
  }
  st.bind_text(1, src.url);
  st.bind_int(2, src.priority);
  st.bind_int(3, src.enabled ? 1 : 0);
  sqlite3_step(st.get());
}

std::vector<calendar_source> sqlite_backend::list_calendar_sources() const {
  std::vector<calendar_source> result;
  sqlite_stmt st(db_, "SELECT url, priority, enabled FROM calendar_sources ORDER BY priority DESC, url;");
  if (!st) {
    return result;
  }
  while (step_row(st.get())) {
    calendar_source src;
    src.url = reinterpret_cast<const char*>(sqlite3_column_text(st.get(), 0));
    src.priority = sqlite3_column_int(st.get(), 1);
    src.enabled = sqlite3_column_int(st.get(), 2) != 0;
    result.push_back(std::move(src));
  }
  return result;
}

void sqlite_backend::remove_calendar_source(const std::string& url) {
  sqlite_stmt st(db_, "DELETE FROM calendar_sources WHERE url=?1;");
  if (!st) {
    return;
  }
  st.bind_text(1, url);
  sqlite3_step(st.get());
}

void sqlite_backend::clear() {
  std::string err;
  if (!exec("BEGIN;", err)) {
    log()->error("storage: clear BEGIN failed: {}", err);
    return;
  }
  const bool ok1 = exec("DELETE FROM accounts;", err);
  const bool ok2 = ok1 && exec("DELETE FROM calendar_sources;", err);
  if (!ok2) {
    log()->error("storage: clear failed: {}", err);
    exec("ROLLBACK;", err);
    return;
  }
  exec("COMMIT;", err);
}

}  // namespace divoomdev::storage