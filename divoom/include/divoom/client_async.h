#pragma once
#include "common/common.hpp"
#include <string>
#include <vector>

#include "client.h"

namespace divoomdev::divoom {

class client_async final : public client {
 public:
  client_async(std::string host = common::DIVOOMDEV_HOST);
  ~client_async();

  std::vector<common::device> get_devices() override;

 private:
  void add_request();
};
}  // namespace divoomdev::divoom
