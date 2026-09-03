#pragma once
#include "../command.h"
#include "divoom/clock_config.h"
#include "divoom/user_info.h"
#include "handler.hpp"


namespace divoomdev::divoom::handlers::clock {

struct get_config_command : command, user_info {
  int DeviceId = 0;
  int ClockId = 0;
  std::string DeviceType = "Frame";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(get_config_command, Command, DeviceId, DeviceType, ClockId, UserId, Token);

struct get_config : handler<get_config_command, clock_config> {
  get_config(int device_id, int clock_id, const user_info& user);
  ~get_config() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::clock
