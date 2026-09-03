#pragma once
#include "../command.h"
#include "common/display_rendered.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::custom_control {

struct enter_command : command {
  /// 	Display base image, it will is url and The resolution of the image must be 800 * 1280
  std::string BackgroudImageAddr;
  /// 1: “BackgroudImageAddr” will be a local file(“/userdata/clock_bg.jpg”); 0: it will be url file
  int BackgroudImageLocalFlag;

  /// Screen display widgets
  common::display_rendered_list DispList;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(enter_command, Command, BackgroudImageAddr, BackgroudImageLocalFlag, DispList);

struct enter : handler<enter_command> {
  enter(common::display_rendered_list display_list,
        std::string backgroud_image_addr,
        int backgroud_image_local_flag = 1);
  ~enter() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::custom_control
