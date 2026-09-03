#pragma once

namespace divoomdev::divoom {

struct user_info {
  int Token = 0;
  int UserId = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(user_info, Token, UserId);

}  // namespace divoomdev::divoom
