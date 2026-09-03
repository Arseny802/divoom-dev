#pragma once
#include "../command.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::commands {

struct patch_clock_info_command : command {
  bool UseCurrentDisplayClock = true;
  common::display_rendered_list ItemPatchList;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(patch_clock_info_command, Command, UseCurrentDisplayClock, ItemPatchList);

struct patch_clock_info : handler<patch_clock_info_command> {
  patch_clock_info(common::display_rendered_list patch_list);
  ~patch_clock_info() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::commands
