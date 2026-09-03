#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct index : handler<command> {
  index();
  ~index() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::commands
