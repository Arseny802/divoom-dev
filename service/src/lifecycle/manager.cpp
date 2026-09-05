#include "divoom/divoom.hpp"
#include "hare/config_default.h"
#include "hare/hare_loggers.h"
#include "ics/ics.hpp"
#include "storage/storage.hpp"

#include "core/service.h"
#include "manager.h"
#include "setup.h"

#include <boost/process.hpp>

#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#endif

namespace fs = std::filesystem;

namespace divoomdev::service::lifecycle {

namespace {

bool ensure_directory(const std::string_view path) {
  try {
    if (!fs::exists(path)) {
      if (!fs::create_directories(path)) {
        return false;
      }
#if defined(_WIN32) || defined(_WIN64)
      SetFileAttributesA(path.data(), FILE_ATTRIBUTE_NORMAL);
#endif
    }
    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace

// ======================== Common ========================

bool manager::is_console_mode(int argc, char* argv[]) {
  std::ignore = argv;
  return argc > 1;
}

void manager::run_console(core::service* core) {
  log()->info("Running in console mode");
  core->run();
}

void initialize_logging(const std::string_view path = setup::LOG_DIR) {
  hare::config_ptr cfg = std::make_unique<hare::config_default>(PROJECT_NAME, MODULE_NAME);
  cfg->set_log_path(path.data());
  hare::register_logger(std::move(cfg));
  std::ignore = divoomdev::divoom::get_logger(path.data());
  std::ignore = divoomdev::storage::get_logger(path.data());
  std::ignore = divoomdev::ics::get_logger(path.data());

  log()->info("Application '{}' (version {}) started. PID: {}", PROJECT_NAME, VERSION, boost::this_process::get_id());
}

// ======================== Entry Point ========================

int manager::run(int argc, char* argv[]) {
  initialize_logging();

  // Ensure data and log directories exist
  const auto& data_dir = setup::DATA_DIR;
  const auto& log_dir = setup::LOG_DIR;
  if (!ensure_directory(data_dir)) {
    log()->error("Cannot create data directory: {}", data_dir);
    return EXIT_FAILURE;
  }
  if (!ensure_directory(log_dir)) {
    log()->error("Cannot create log directory: {}", log_dir);
    return EXIT_FAILURE;
  }

  auto core = create_service_core();
  core->set_update_interval(setup::UPDATE_INTERVAL);

  if (is_console_mode(argc, argv)) {
    run_console(core.get());
    return EXIT_SUCCESS;
  }

  log()->info("Running as service");
  if (!is_running_as_service()) {

    log()->info("Not running as service, auto-registering...");
    if (!check_admin_privileges()) {
      log()->error("Cannot register service: insufficient privileges.");
      return EXIT_FAILURE;
    }
    if (!auto_register_service()) {
      log()->error("Failed to auto-register service.");
      return EXIT_FAILURE;
    }
    log()->info("Service registered. Exiting.");
    return EXIT_SUCCESS;
  }

  run_service(std::move(core));
  return EXIT_SUCCESS;
}

}  // namespace divoomdev::service::lifecycle
