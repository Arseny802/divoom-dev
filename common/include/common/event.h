#pragma once
#include <chrono>
#include <vector>

namespace divoomdev::common {

struct event {
  std::string summary;
  std::string description;
  std::string uid;
  std::string location;
  int sequence = 0;

  std::chrono::time_point<std::chrono::system_clock> start;
  std::chrono::time_point<std::chrono::system_clock> end;

  std::chrono::time_point<std::chrono::system_clock> created;
  std::chrono::time_point<std::chrono::system_clock> last_modified;
  std::chrono::time_point<std::chrono::system_clock> stamp;
};
using event_list = std::vector<event>;

}  // namespace divoomdev::common
