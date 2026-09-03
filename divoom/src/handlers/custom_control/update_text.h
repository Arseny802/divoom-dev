#pragma once
#include "../command.h"
#include "common/display.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::custom_control {

struct update_text_command : command {
  common::display_list DispList;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(update_text_command, Command, DispList);

struct update_text : handler<update_text_command> {
  update_text(common::display_list display_list);
  ~update_text() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::custom_control
