#pragma once
#include <atomic>

namespace divoomdev::service::lifecycle {

/// Глобальное состояние службы (общее для windows_service.cpp и service.cpp)
inline std::atomic<bool>& g_service_running() {
  static std::atomic<bool> instance{true};
  return instance;
}

inline std::atomic<bool>& g_service_paused() {
  static std::atomic<bool> instance{false};
  return instance;
}

}  // namespace divoomdev::service::lifecycle
