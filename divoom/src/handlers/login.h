#pragma once
#include "command.h"
#include "common/device.h"
#include "divoom/user_info.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers {

struct login_command : command {
  std::string Email;
  std::string Password;
  std::string Language = "ru";
  std::string TimeZone = "+3";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(login_command, Command, Email, Password, Language, TimeZone);

struct login : handler<login_command, user_info> {
  login(std::string email, std::string password);
  ~login() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;

 private:
  std::string compute_md5(const std::string& input);
};

}  // namespace divoomdev::divoom::handlers
