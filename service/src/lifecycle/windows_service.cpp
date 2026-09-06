#include <memory>
#include <stdexcept>
#include <string>
#if defined(_WIN32) || defined(_WIN64)

// clang-format off
#include "setup.h"
#include "core/service.h"
#include "lifecycle/manager.h"
#include "lifecycle/service_state.h"
#include "storage/storage.hpp"

#include <aclapi.h>
#include <atomic>
#include <processthreadsapi.h>
#include <thread>
#include <windows.h>
// clang-format on

namespace divoomdev::service::lifecycle {

// ======================== Windows Service ========================

static SERVICE_STATUS g_service_status{};
static SERVICE_STATUS_HANDLE g_service_status_handle = nullptr;
static std::unique_ptr<core::service> g_service_core_ptr = nullptr;

void WINAPI service_ctrl_handler(DWORD ctrl) {
  SERVICE_STATUS status{};
  status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;

  switch (ctrl) {
  case SERVICE_CONTROL_STOP:
  case SERVICE_CONTROL_SHUTDOWN:
    g_service_paused() = false;
    if (g_service_core_ptr) {
      g_service_core_ptr->stop();
    }
    status.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(g_service_status_handle, &status);
    log()->info("Service stopping...");
    break;

  case SERVICE_CONTROL_PAUSE:
    g_service_paused() = true;
    status.dwCurrentState = SERVICE_PAUSE_PENDING;
    SetServiceStatus(g_service_status_handle, &status);
    if (g_service_core_ptr) {
      g_service_core_ptr->pause();
    }
    status.dwCurrentState = SERVICE_PAUSED;
    SetServiceStatus(g_service_status_handle, &status);
    log()->info("Service paused");
    break;

  case SERVICE_CONTROL_CONTINUE:
    g_service_paused() = false;
    status.dwCurrentState = SERVICE_CONTINUE_PENDING;
    SetServiceStatus(g_service_status_handle, &status);
    if (g_service_core_ptr) {
      g_service_core_ptr->resume();
    }
    status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_service_status_handle, &status);
    log()->info("Service resumed");
    break;

  default: break;
  }
}

void WINAPI service_main(DWORD argc, LPWSTR* argv) {
  std::ignore = argc;
  std::ignore = argv;

  g_service_status_handle = RegisterServiceCtrlHandlerW(setup::SERVICE_NAME_W, service_ctrl_handler);

  if (!g_service_status_handle) {
    log()->error("RegisterServiceCtrlHandlerW failed: {}", GetLastError());
    return;
  }

  g_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  g_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
  g_service_status.dwCurrentState = SERVICE_START_PENDING;
  SetServiceStatus(g_service_status_handle, &g_service_status);

  log()->info("Windows service starting...");

  // Запускаем рабочий цикл в отдельном потоке
  std::thread worker([=]() {
    if (g_service_core_ptr) {
      g_service_status.dwCurrentState = SERVICE_RUNNING;
      SetServiceStatus(g_service_status_handle, &g_service_status);
      log()->info("Windows service started");

      g_service_core_ptr->run();

      log()->info("Service core run() returned");
    }
  });

  // Ждём остановки
  while (g_service_running()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  log()->info("Waiting for worker thread to finish...");
  worker.join();
  log()->info("Worker thread joined");

  g_service_status.dwCurrentState = SERVICE_STOPPED;
  SetServiceStatus(g_service_status_handle, &g_service_status);
  log()->info("Windows service stopped");
}

std::unique_ptr<core::service> manager::create_service_core() {
  AUTOTRACEF;

  auto storage = storage::open_storage(storage::backend_type::sqlite,
                                       std::string(setup::DATA_DIR) + "/" + std::string(setup::DATABASE_FILE));
  if (!storage) {
    throw std::runtime_error("storage: failed to open storage at '" + std::string(setup::DATA_DIR) + "/" +
                             std::string(setup::DATABASE_FILE) + "'");
  }
  return std::make_unique<core::service>(std::move(storage));
}

bool manager::is_running_as_service() {
  // Services run in Session 0; interactive processes run in Session > 0.
  DWORD sessionId = 0;
  if (!ProcessIdToSessionId(GetCurrentProcessId(), &sessionId)) {
    log()->warning("ProcessIdToSessionId failed: {}", GetLastError());
    return false;  // Fallback to console mode on error
  }
  log()->debug("Session ID: {}", sessionId);
  return sessionId == 0;
}

bool manager::check_admin_privileges() {
  AUTOTRACEF;
  SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
  if (!scm) {
    log()->error("OpenSCManager failed (error {}): insufficient privileges. "
                 "Please run as Administrator.",
                 GetLastError());
    return false;
  }
  CloseServiceHandle(scm);
  return true;
}

bool manager::auto_register_service() {
  AUTOTRACEF;
  wchar_t exe_path_w[MAX_PATH] = {};
  const DWORD len = GetModuleFileNameW(nullptr, exe_path_w, MAX_PATH);
  if (len == 0 || len == MAX_PATH) {
    log()->error("GetModuleFileNameW failed: {}", GetLastError());
    return false;
  }
  std::wstring exe_path_wide(exe_path_w);
  const std::string exe_path(exe_path_wide.begin(), exe_path_wide.end());

  std::string service_name = setup::SERVICE_NAME;

  log()->info("Auto-registering service: {}", service_name);
  log()->info("Executable path: {}", exe_path);

  SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
  if (!scm) {
    log()->error("OpenSCManager failed: {}", GetLastError());
    return false;
  }

  SC_HANDLE svc = OpenServiceW(scm, setup::SERVICE_NAME_W, SERVICE_QUERY_CONFIG);
  if (svc) {
    if (!ChangeServiceConfigW(svc,
                              SERVICE_NO_CHANGE,
                              SERVICE_AUTO_START,
                              SERVICE_NO_CHANGE,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr)) {
      log()->warning("Could not set auto-start on existing service");
    }
    CloseServiceHandle(svc);
    log()->info("Service already exists, ensuring auto-start");
  } else {
    svc = CreateServiceW(scm,
                         setup::SERVICE_NAME_W,
                         setup::SERVICE_DISPLAY_NAME_W,
                         SERVICE_ALL_ACCESS,
                         SERVICE_WIN32_OWN_PROCESS,
                         SERVICE_AUTO_START,
                         SERVICE_ERROR_NORMAL,
                         exe_path_w,
                         nullptr,
                         nullptr,
                         nullptr,
                         nullptr,
                         nullptr);

    if (!svc) {
      log()->error("CreateService failed: {}", GetLastError());
      CloseServiceHandle(scm);
      return false;
    }

    SERVICE_DESCRIPTIONW desc;
    desc.lpDescription = const_cast<LPWSTR>(setup::SERVICE_DESCRIPTION_W);
    ChangeServiceConfig2W(svc, SERVICE_CONFIG_DESCRIPTION, &desc);
    CloseServiceHandle(svc);
    log()->info("Service created successfully");
  }

  CloseServiceHandle(scm);

  // Запускаем службу
  scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
  if (!scm) {
    log()->error("OpenSCManager failed: {}", GetLastError());
    return false;
  }

  svc = OpenServiceW(scm, setup::SERVICE_NAME_W, SERVICE_START);
  if (!svc) {
    log()->error("OpenService failed: {}", GetLastError());
    CloseServiceHandle(scm);
    return false;
  }

  if (!StartServiceW(svc, 0, nullptr)) {
    DWORD err = GetLastError();
    if (err != ERROR_SERVICE_ALREADY_RUNNING) {
      log()->error("StartService failed: {}", err);
      CloseServiceHandle(svc);
      CloseServiceHandle(scm);
      return false;
    }
    log()->info("Service is already running");
  } else {
    log()->info("Service started successfully");
  }

  CloseServiceHandle(svc);
  CloseServiceHandle(scm);
  return true;
}

void manager::run_service(std::unique_ptr<core::service>&& core) {
  AUTOFLUSH_ALL;
  AUTOTRACEF;
  g_service_core_ptr = std::move(core);

  SERVICE_TABLE_ENTRYW service_table[] = {{const_cast<LPWSTR>(setup::SERVICE_NAME_W), service_main},
                                          {nullptr, nullptr}};

  if (!StartServiceCtrlDispatcherW(service_table)) {
    log()->error("StartServiceCtrlDispatcher failed: {}", GetLastError());
  }
}

}  // namespace divoomdev::service::lifecycle
#endif  // if defined(_WIN32) || defined(_WIN64)
