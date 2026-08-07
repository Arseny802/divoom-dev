#pragma once
#include "command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct command_mirror : command {
  int mode = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(command_mirror, command, mode);

struct mirror : handler<command_mirror> {
  mirror(int mode = 0);
  ~mirror() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override { return true; };
};

}  // namespace divoomdev::divoom::handlers::commands
