#pragma once
#include "command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct command_timeformat : command {
  int mode = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(command_timeformat, command, mode);

struct time_format : handler<command_timeformat> {
  time_format(int mode = 0);
  ~time_format() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override { return true; };
};

}  // namespace divoomdev::divoom::handlers::commands
