#pragma once
#include "common/device.h"

#include <string>

namespace divoomdev::divoom {
class client;
class user_info;
}  // namespace divoomdev::divoom

namespace divoomdev::service::core {

class device_updater {
 public:
  device_updater() noexcept;
  device_updater(std::string login, std::string password) noexcept;
  ~device_updater();

  void update_credentials(std::string login, std::string password) noexcept;

  /// Обновляет все найденные устройства заданным текстом.
  /// @throws std::runtime_error если устройства не найдены
  void update(const std::string& text);

 private:
  void update_device(int device_id, const std::string& text);
  bool login();
  bool get_devices();

  std::string login_{};
  std::string password_{};
  std::size_t previous_text_hash_{};
  std::unique_ptr<divoom::client> client_;
  std::unique_ptr<divoom::user_info> user_info_;
  std::unique_ptr<common::device_list> devices_;
};

}  // namespace divoomdev::service::core
