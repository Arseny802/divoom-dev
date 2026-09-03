#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::clock {

struct select_id_command : command {
  int ClockId = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(select_id_command, Command, ClockId);

struct select_id : handler<select_id_command> {
  select_id(int clock_id);
  ~select_id() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::clock
