#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct command_high_light : command {
  int Mode = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(command_high_light, Command, Mode);

struct high_light : handler<command_high_light> {
  high_light(int mode = 0);
  ~high_light() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::commands
