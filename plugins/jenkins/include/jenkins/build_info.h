#pragma once

#include <string>
#include <vector>

namespace divoomdev::jenkins {

struct build_info {
  int number;
  std::string status;
  std::string result;
  bool building;
  uint64_t timestamp;
};

using build_info_list = std::vector<build_info>;

}  // namespace divoomdev::jenkins
