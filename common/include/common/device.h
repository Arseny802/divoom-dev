#pragma once

namespace divoomdev::common {

struct device {
  std::string name;
  int id;
  std::string private_ip;
  std::string mac;
  int hardware;
};

}  // namespace divoomdev::common
