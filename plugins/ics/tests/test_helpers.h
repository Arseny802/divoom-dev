#pragma once
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace ics_test {

/// Reads a file located under the tests source directory.
inline std::string read_file(const std::string& rel_path) {
  std::filesystem::path base = std::filesystem::path(__FILE__).parent_path();
  std::ifstream f(base / rel_path, std::ios::binary);
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

/// Returns the raw content of a fixture file under fixtures/.
inline std::string fixture(const std::string& name) {
  return read_file("fixtures/" + name);
}

/// Formats a time point in local time as "%Y-%m-%d %H:%M:%S".
inline std::string fmt_dt(std::chrono::time_point<std::chrono::system_clock> tp) {
  auto t = std::chrono::system_clock::to_time_t(tp);
  struct tm tm;
  localtime_s(&tm, &t);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
  return std::string(buf);
}

/// Formats a time point in UTC as "%Y%m%dT%H%M%S" (for absolute-time checks).
inline std::string utc_dt(std::chrono::time_point<std::chrono::system_clock> tp) {
  auto t = std::chrono::system_clock::to_time_t(tp);
  struct tm tm;
  _gmtime64_s(&tm, &t);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", &tm);
  return std::string(buf);
}

/// Builds an absolute moment from wall-clock local time.
inline std::chrono::time_point<std::chrono::system_clock> wall_clock(int y, int mo, int d, int h, int mi, int s) {
  struct tm tm = {};
  tm.tm_year = y - 1900;
  tm.tm_mon = mo - 1;
  tm.tm_mday = d;
  tm.tm_hour = h;
  tm.tm_min = mi;
  tm.tm_sec = s;
  tm.tm_isdst = -1;
  return std::chrono::system_clock::from_time_t(mktime(&tm));
}

}  // namespace ics_test