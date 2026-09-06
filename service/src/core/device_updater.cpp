#include "device_updater.h"
#include "divoom/client.h"
#include "divoom/exceptions.h"
#include "divoom/user_info.h"
#include <memory>
#include <utility>

namespace divoomdev::service::core {

device_updater::device_updater() noexcept: device_updater({}, {}) { }
device_updater::device_updater(std::string login, std::string password) noexcept
    : login_(std::move(login)),
      password_(std::move(password)) {
  client_ = std::make_unique<divoom::client>();
}

device_updater::~device_updater() = default;

void device_updater::update_credentials(std::string login, std::string password) noexcept {
  AUTOTRACEF;
  bool need_reauth = false;
  need_reauth |= login_ != std::exchange(login_, std::move(login));
  need_reauth |= password_ != std::exchange(password_, std::move(password));
  if (need_reauth) {
    log()->info("Credentials changed, reauthenticating...");
    this->login();
    get_devices();
  } else {
    log()->debug("Credentials not changed, skipping reauthentication");
  }
}

void device_updater::update(const std::string& text) {
  AUTOMEASUREF;

  if (std::hash<std::string>{}(text) == previous_text_hash_) {
    log()->info("Text hash not changed, skipping update");
    return;
  }

  if (!devices_ || !user_info_) {
    log()->error("No devices or user info, skipping update");
    return;
  }

  const auto retry_with_login = [this, &text](const std::exception& e) {
    AUTOTRACEF;
    log()->warning("Token error: {}. Retrying with login...", e.what());
    if (login()) {
      get_devices();
      update(text);
    }
  };

  try {
    for (const auto& dev: *devices_) {
      update_device(dev.id, text);
    }
    previous_text_hash_ = std::hash<std::string>{}(text);
  } catch (const divoom::WrongPasswordException& e) {
    retry_with_login(e);
  } catch (const divoom::WrongUserException& e) {
    retry_with_login(e);
  } catch (const divoom::TokenExpiredException& e) {
    retry_with_login(e);
  } catch (const std::exception& e) {
    log()->error("Error: {}", e.what());
  } catch (...) {
    log()->critical("Unknown error while updating divoom devices text");
  }
}

void device_updater::update_device(int device_id, const std::string& text) {
  const int current_clock_id = 598;  // TODO: get current clock id
  auto clock_config = client_->get_clock_config(device_id, current_clock_id, *user_info_);

  for (auto& item: clock_config.ItemList) {
    if (item.ItemType == 25) {
      item.ItemValue = text;
    }
  }

  log()->info("set_clock_config: {}",
              client_->set_clock_config(device_id, current_clock_id, *user_info_, clock_config));
}

bool device_updater::login() {
  AUTOTRACEF;
  try {
    user_info_ = std::make_unique<divoom::user_info>(client_->login(login_, password_));
  } catch (const std::exception& e) {
    log()->error("Error: {}", e.what());
    return false;
  } catch (...) {
    log()->critical("Unknown error while logging in to divoom");
    return false;
  }

  log()->info("Token {}, User ID {}", user_info_->Token, user_info_->UserId);
  return true;
}

bool device_updater::get_devices() {
  AUTOTRACEF;
  try {
    devices_ = std::make_unique<common::device_list>(client_->get_devices());
  } catch (const std::exception& e) {
    log()->error("Error: {}", e.what());
    return false;
  } catch (...) {
    log()->critical("Unknown error while getting devices");
    return false;
  }

  if (devices_->empty()) {
    log()->error("No devices found in LAN!!!");
  }

  for (const auto& dev: *devices_) {
    log()->info("Device '{}'':"
                "\n\tID = {}\n\tip = {}\n\tmac = {}\n\thardware = {}\n",
                dev.name,
                dev.id,
                dev.private_ip,
                dev.mac,
                dev.hardware);
  }
  return true;
}

}  // namespace divoomdev::service::core
