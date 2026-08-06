#pragma once
#include <string>

namespace divoomdev::divoom::handlers::commands {

struct command {
  std::string command;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(command, command);

}  // namespace divoomdev::divoom::handlers::commands
