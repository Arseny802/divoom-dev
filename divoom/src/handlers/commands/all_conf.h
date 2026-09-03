#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct all_conf_command : command {
  std::string ChannelType;
  int ChannelId = 0;
};

struct all_conf : handler<all_conf_command> {
  all_conf();
  ~all_conf() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::commands
