#pragma once
#include <vector>

namespace divoomdev::common {

struct device {
  std::string name;
  int id;
  std::string private_ip;
  std::string mac;
  int hardware;
};

using device_list = std::vector<common::device>;

}  // namespace divoomdev::common
