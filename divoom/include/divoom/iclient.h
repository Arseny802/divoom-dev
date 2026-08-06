#pragma once
#include "common/common.hpp"
#include <string>
#include <vector>

namespace divoomdev::divoom {
class iclient {
 public:
  virtual ~iclient() = default;

  virtual std::vector<common::device> get_devices() = 0;
};
}  // namespace divoomdev::divoom
