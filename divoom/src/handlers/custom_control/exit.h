#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::custom_control {

struct exit : handler<command> {
  exit();
  ~exit() override;

  std::string get_path(const std::string_view host) const noexcept override;
};

}  // namespace divoomdev::divoom::handlers::custom_control
