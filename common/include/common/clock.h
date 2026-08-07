#pragma once
#include <vector>

namespace divoomdev::common {

struct clock {
  /// ClockId
  int id;
  /// ClockName
  std::string name;
  /// ClockType
  int type;
  /// ImagePixelId
  std::string image_pixel_id;
  /// Position
  int position;
};

using clock_list = std::vector<clock>;

}  // namespace divoomdev::common
