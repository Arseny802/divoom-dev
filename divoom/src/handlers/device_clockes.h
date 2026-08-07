#pragma once
#include "common/clock.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers {

struct device_clockes_request {
  int DeviceId = 0;
  int StartNum = 1;
  int EndNum = 100;
  std::string DeviceType = "Frame";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(device_clockes_request, DeviceId, StartNum, EndNum, DeviceType);

struct device_clockes : handler<device_clockes_request, common::clock_list> {
  device_clockes(device_clockes_request request = {});
  ~device_clockes() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers
