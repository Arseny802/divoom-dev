#pragma once
#include <memory>

namespace divoomdev::storage {
class i_settings_storage;
}
namespace divoomdev::service::core {
class service;
}

namespace divoomdev::service::lifecycle {

/// Управляет жизненным циклом приложения:
/// - Windows: регистрация и запуск как Windows Service
/// - Linux: регистрация и запуск как systemd daemon
/// - Console: прямой запуск без регистрации
class manager {
 public:
  /// Определяет режим запуска и выполняет его.
  /// @return код завершения
  static int run(int argc, char* argv[]);

 private:
  static bool is_console_mode(int argc, char* argv[]);
  static void run_console(core::service* core);

  static std::unique_ptr<core::service> create_service_core();
  static bool is_running_as_service();
  static bool auto_register_service();
  static bool check_admin_privileges();
  static void run_service(std::unique_ptr<core::service>&& core);

  std::unique_ptr<storage::i_settings_storage> storage_;
};

}  // namespace divoomdev::service::lifecycle
