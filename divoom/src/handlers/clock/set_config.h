#pragma once
#include "../command.h"
#include "divoom/clock_config.h"
#include "divoom/user_info.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::clock {

struct set_config_command : command, user_info, clock_config {
  int DeviceId;
  int ClockId;

  int LcdIndependence = 0;
  int LcdIndex = 0;
  int PageIndex = 0;
  int ParentClockId = 0;
  std::string ParentItemId = "ru";
  std::string Language = "ru";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(set_config_command,
                                   Command,
                                   DeviceId,
                                   ClockId,
                                   UserId,
                                   Token,
                                   ItemList,
                                   LcdIndependence,
                                   LcdIndex,
                                   PageIndex,
                                   ParentClockId,
                                   ParentItemId,
                                   Language);

struct set_config : handler<set_config_command, clock_config> {
  set_config(int device_id, int clock_id, const user_info& user, clock_config::item_list_t item_list);
  ~set_config() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::clock
