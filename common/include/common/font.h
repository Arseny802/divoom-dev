#pragma once
#include <vector>

namespace divoomdev::common {

struct font {
  int id;
  int type;
  std::string url;
  std::string char_set;
  std::string encryption;
};

using font_list = std::vector<common::font>;

}  // namespace divoomdev::common
