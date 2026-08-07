#pragma once
#include "command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct clock_select_id_command : command {
  int ClockId = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(clock_select_id_command, command, ClockId);

struct clock_select_id : handler<clock_select_id_command> {
  clock_select_id(int clock_id);
  ~clock_select_id() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override { return true; };
};

}  // namespace divoomdev::divoom::handlers::commands
