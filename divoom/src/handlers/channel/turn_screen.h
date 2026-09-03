#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct turn_screen_command : command {
  int OnOff = 1;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(turn_screen_command, Command, OnOff);

struct turn_screen : handler<turn_screen_command> {
  turn_screen(int turn_on = 1);
  ~turn_screen() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::commands
