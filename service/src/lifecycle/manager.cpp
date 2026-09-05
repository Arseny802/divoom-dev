#include "lifecycle/manager.h"
#include "core/service.h"
#include "setup.h"

#include <boost/process.hpp>

#include "divoom/divoom.hpp"
#include "ics/ics.hpp"
#include "storage/storage.hpp"

namespace divoomdev::service::lifecycle {

// ======================== Common ========================

bool manager::is_console_mode(int argc, char* argv[]) {
  std::ignore = argv;
  return argc > 1;
}

void manager::run_console(core::service* core) {
  log()->info("Running in console mode");
  core->run();
}

void initialize_logging() {
  log()->info("Application started. PID: {}", boost::this_process::get_id());
  std::ignore = divoomdev::divoom::get_logger();
  std::ignore = divoomdev::storage::get_logger();
  std::ignore = divoomdev::ics::get_logger();
}

// ======================== Entry Point ========================

int manager::run(int argc, char* argv[]) {
  initialize_logging();
  auto core = create_service_core();
  core->set_update_interval(setup::UPDATE_INTERVAL);

  if (is_console_mode(argc, argv)) {
    run_console(core.get());
    return EXIT_SUCCESS;
  }

  log()->info("Running as service");
  if (!is_running_infinity()) {

    log()->info("Not running as service, auto-registering...");
    if (!auto_register_service()) {
      log()->error("Failed to auto-register service. Run as administrator.");
      return EXIT_FAILURE;
    }
    log()->info("Service registered. Exiting.");
    return EXIT_SUCCESS;
  }

  run_service(std::move(core));
  return EXIT_SUCCESS;
}

}  // namespace divoomdev::service::lifecycle
