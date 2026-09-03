#pragma once
#include <vector>

namespace divoomdev::common {

struct display {
  /// Display element ID. If this ID has been added before, we will overwrite the previous element. Otherwise, it is a
  /// new element added, greater than 0
  int ID = 0;
  /// display content when type is “Text”.
  std::string TextMessage{};
};
using display_list = std::vector<display>;

#ifdef NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(display, ID, TextMessage);
#endif

}  // namespace divoomdev::common
