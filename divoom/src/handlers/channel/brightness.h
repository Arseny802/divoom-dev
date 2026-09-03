#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct command_brightness : command {
  int Brightness = 100;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(command_brightness, Command, Brightness);

struct brightness : handler<command_brightness> {
  brightness(int brightness = 100);
  ~brightness() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::commands
