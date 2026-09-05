#if !defined(_WIN32) && !defined(_WIN64)

// clang-format off
#include <string>

#include "service/lifecycle_manager.h"
#include "service/service_core.h"
#include "storage/storage.hpp"

#include <boost/process.hpp>
#include <filesystem>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
// clang-format on

namespace bp = boost::process;
namespace fs = std::filesystem;

namespace divoomdev::service::lifecycle {

// ======================== Linux/Unix Daemon ========================

static std::atomic<bool> g_daemon_running{true};

void daemon_signal_handler(int signum) {
  std::ignore = signum;
  g_daemon_running = false;
  log()->info("Received signal, shutting down...");
}

std::unique_ptr<core::service> manager::create_service_core() {
  auto storage = storage::open_storage(storage::backend_type::sqlite,
                                       std::string(setup::DATA_DIR) + "/" + setup::DATABASE_FILE);
  return std::make_unique<core::service>(std::move(storage));
}

bool manager::is_running_as_service() {
  const char* pid = getenv("SYSTEMD_EXEC_PID");
  return pid != nullptr && pid[0] != '\0';
}

bool manager::check_admin_privileges() {
  if (geteuid() != 0) {
    log()->error(
        "Insufficient privileges. Please run as root (use sudo).");
    return false;
  }
  return true;
}

bool manager::auto_register_service() {
  auto exe_path = fs::current_path().string();
  std::string service_name = setup::SERVICE_NAME + ".service";
  std::string unit_path = "/etc/systemd/system/" + service_name;

  log()->info("Auto-creating systemd unit: {}", unit_path);

  std::string unit_content = "# Divoom Service - auto-generated\n"
                             "[Unit]\n"
                             "Description=" + setup::SERVICE_DESCRIPTION + "\n"
                             "After=network.target\n"
                             "\n"
                             "[Service]\n"
                             "Type=simple\n"
                             "Environment=DIVOOMDEV_DATA_DIR=" + std::string(setup::DATA_DIR) + "\n"
                             "Environment=DIVOOMDEV_LOG_DIR=" + std::string(setup::LOG_DIR) + "\n"
                             "ExecStart=" + exe_path + "\n"
                             "Restart=on-failure\n"
                             "RestartSec=5\n"
                             "StandardOutput=journal\n"
                             "StandardError=journal\n"
                             "\n"
                             "[Install]\n"
                             "WantedBy=multi-user.target\n";

  // Записываем unit-файл
  {
    bp::ipstream output;
    bp::child c("sudo tee " + unit_path, bp::std_out > output);
    c << unit_content;
    c.wait();

    if (c.exit_code() != 0) {
      std::string err;
      while (std::getline(output, err)) {
        log()->error("sudo tee: {}", err);
      }
      log()->error("Failed to create systemd unit file");
      return false;
    }
  }

  // Перечитываем конфигурацию
  {
    bp::ipstream output;
    bp::child c("sudo systemctl daemon-reload", bp::std_out > output);
    c.wait();

    if (c.exit_code() != 0) {
      std::string err;
      while (std::getline(output, err)) {
        log()->error("daemon-reload: {}", err);
      }
      log()->error("Failed to reload systemd daemon");
      return false;
    }
    log()->info("Systemd daemon reloaded");
  }

  // Включаем автозапуск
  {
    bp::ipstream output;
    bp::child c("sudo systemctl enable " + service_name, bp::std_out > output);
    c.wait();

    if (c.exit_code() != 0) {
      std::string err;
      while (std::getline(output, err)) {
        log()->error("enable: {}", err);
      }
      log()->error("Failed to enable systemd service");
      return false;
    }
    log()->info("Systemd service enabled");
  }

  // Запускаем
  {
    bp::ipstream output;
    bp::child c("sudo systemctl start " + service_name, bp::std_out > output);
    c.wait();

    if (c.exit_code() != 0) {
      std::string err;
      while (std::getline(output, err)) {
        log()->error("start: {}", err);
      }
      log()->error("Failed to start systemd service");
      return false;
    }
    log()->info("Systemd service started");
  }

  return true;
}

void manager::run_service(std::unique_ptr<core::service>&& core) {
  log()->info("Becoming daemon (fork+setsid)");

  pid_t pid = fork();
  if (pid < 0) {
    log()->error("Fork failed: {}", strerror(errno));
    return;
  }

  if (pid > 0) {
    log()->info("Parent exiting, child PID: {}", pid);
    return;
  }

  if (setsid() < 0) {
    log()->error("setsid failed: {}", strerror(errno));
    return;
  }

  pid = fork();
  if (pid < 0) {
    log()->error("Second fork failed: {}", strerror(errno));
    return;
  }

  if (pid > 0) {
    log()->info("Second parent exiting");
    return;
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_WRONLY);

  signal(SIGTERM, daemon_signal_handler);
  signal(SIGINT, daemon_signal_handler);

  log()->info("Daemon running with PID: {}", getpid());
  core->run();
}

}  // namespace divoomdev::service::lifecycle

#endif  // !defined(_WIN32) && !defined(_WIN64)
