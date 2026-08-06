#pragma once
#include "command.h"
#include "divoom/handler.h"

namespace divoomdev::divoom::handlers::commands {

struct reboot : handler<command> {
  reboot();
  ~reboot() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override { return true; };
};

}  // namespace divoomdev::divoom::handlers::commands
