#pragma once
#include "command.h"
#include "divoom/handler.h"

namespace divoomdev::divoom::handlers::commands {

struct command_brightness : command {
  int brightness = 100;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(command_brightness, command, brightness);

struct brightness : handler<command_brightness> {
  brightness(int brightness = 100);
  ~brightness() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override { return true; };
};

}  // namespace divoomdev::divoom::handlers::commands
