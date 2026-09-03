#pragma once
#include <chrono>
#include <string>
#include <vector>

namespace divoomdev::common {

struct alarm {
  std::chrono::minutes trigger_minutes;  // отрицательное значение = за N минут до
  std::string action;                    // DISPLAY, EMAIL, AUDIO
  std::string description;
};

enum class event_status { CONFIRMED, TENTATIVE, CANCELED };

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

  std::string url;
  std::string organizer;
  std::string organizer_name;
  std::string categories;
  event_status status = event_status::CONFIRMED;

  std::vector<alarm> alarms;
};
using event_list = std::vector<event>;

}  // namespace divoomdev::common
