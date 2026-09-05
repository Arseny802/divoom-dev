#pragma once

#include <string>

namespace divoomdev::service::core {

class device_updater {
 public:
  /// Обновляет все найденные устройства заданным текстом.
  /// @throws std::runtime_error если устройства не найдены
  void update(const std::string& text, const std::string& login, const std::string& password);
};

}  // namespace divoomdev::service::core
