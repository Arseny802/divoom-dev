#pragma once
#include <string>

namespace divoomdev::divoom::handlers {

struct command {
  std::string Command;
  std::string ReturnMessage = "";
  int ReturnCode = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(command, Command, ReturnMessage, ReturnCode);

}  // namespace divoomdev::divoom::handlers
