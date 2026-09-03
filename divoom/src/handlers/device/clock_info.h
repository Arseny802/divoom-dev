#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct clock_info_command : command {
  int DeviceId = 300417773;
  int ClockId = 311;
  int ParentItemId = 383923;
  int ParentClockId = 383923;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(clock_info_command, Command, DeviceId, ClockId, ParentItemId, ParentClockId);

struct clock_info : handler<command> {
  clock_info();
  ~clock_info() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::commands
