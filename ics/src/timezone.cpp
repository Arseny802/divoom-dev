#include "timezone.h"

#include "text_utils.h"

#include <sstream>

namespace divoomdev::ics::detail {

int parse_tzoffset(const std::string& s) {
  if (s.size() < 5)
    return kUnknownOffset;
  int sign = 1;
  size_t i = 0;
  if (s[0] == '+') {
    i = 1;
  } else if (s[0] == '-') {
    sign = -1;
    i = 1;
  }
  if (i + 4 > s.size())
    return kUnknownOffset;
  try {
    int hh = std::stoi(s.substr(i, 2));
    int mm = std::stoi(s.substr(i + 2, 2));
    int hours = hh + (mm >= 30 ? 1 : 0);  // округляем получасовые смещения
    return sign * hours;
  } catch (...) {
    return kUnknownOffset;
  }
}

void timezone_resolver::collect_vtimezones(const std::string& ics_content) {
  vtz_offsets_.clear();
  std::istringstream stream(ics_content);
  std::string line;
  bool in_tz = false;
  bool in_std = false;
  std::string cur_tzid;
  int std_offset = kUnknownOffset;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    std::string t = trim(line);
    if (t.rfind("BEGIN:VTIMEZONE", 0) == 0) {
      in_tz = true;
      in_std = false;
      cur_tzid.clear();
      std_offset = kUnknownOffset;
      continue;
    }
    if (!in_tz)
      continue;
    if (t.rfind("END:VTIMEZONE", 0) == 0) {
      if (!cur_tzid.empty() && std_offset != kUnknownOffset)
        vtz_offsets_[cur_tzid] = std_offset;
      in_tz = false;
      in_std = false;
      continue;
    }
    if (t.rfind("BEGIN:STANDARD", 0) == 0) {
      in_std = true;
      continue;
    }
    if (t.rfind("BEGIN:DAYLIGHT", 0) == 0) {
      in_std = false;
      continue;
    }
    if (t.rfind("TZID:", 0) == 0) {
      cur_tzid = t.substr(5);
    } else if (in_std && t.rfind("TZOFFSETTO:", 0) == 0) {
      std_offset = parse_tzoffset(t.substr(11));
    }
  }
}

int timezone_resolver::static_offset(const std::string& tz) {
  static const std::pair<const char*, int> kOffsets[] = {
      {"UTC", 0},
      {"EUROPE/KALININGRAD", 2},
      {"EUROPE/MOSCOW", 3},
      {"EUROPE/VOLGOGRAD", 3},
      {"EUROPE/SAMARA", 4},
      {"ASIA/YEKATERINBURG", 5},
      {"ASIA/BANGKOK", 7},
      {"EUROPE/LONDON", 1},
      // Windows-имена Outlook (запасной вариант)
      {"RUSSIAN STANDARD TIME", 3},
      {"GMT STANDARD TIME", 0},
      {"SE ASIA STANDARD TIME", 7},
      {"EKATERINBURG STANDARD TIME", 5},
      {"RUSSIA TIME ZONE 3", 4},
  };
  std::string key = to_upper(tz);
  for (const auto& e: kOffsets) {
    if (key == e.first)
      return e.second;
  }
  return kUnknownOffset;
}

int timezone_resolver::offset_hours(const std::string& tz) const {
  auto it = vtz_offsets_.find(tz);
  if (it != vtz_offsets_.end())
    return it->second;
  return static_offset(tz);
}

}  // namespace divoomdev::ics::detail