#pragma once
#include "common/clock.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers::clock {

struct get_list_request {
  int DeviceId = 0;
  int StartNum = 1;
  int EndNum = 100;
  std::string DeviceType = "Frame";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(get_list_request, DeviceId, StartNum, EndNum, DeviceType);

struct get_list : handler<get_list_request, common::clock_list> {
  get_list(get_list_request request = {});
  ~get_list() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers::clock
