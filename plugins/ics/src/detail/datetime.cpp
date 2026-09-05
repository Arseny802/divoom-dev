#include "datetime.h"

#include "text_utils.h"

#include <cctype>
#include <ctime>

namespace divoomdev::ics::detail {

datetime_info make_datetime_info(const std::string& params, const std::string& value) {
  datetime_info info;
  info.value = value;

  // Ищем TZID.
  auto tz_pos = params.find("TZID=");
  if (tz_pos != std::string::npos) {
    auto tz_start = tz_pos + 5;
    auto tz_end = params.find(';', tz_start);
    if (tz_end == std::string::npos)
      tz_end = params.size();
    info.timezone = params.substr(tz_start, tz_end - tz_start);
  }

  // VALUE=DATE.
  if (params.find("VALUE=DATE") != std::string::npos)
    info.is_date = true;

  // UTC, если значение оканчивается на Z.
  if (!info.value.empty() && info.value.back() == 'Z')
    info.is_utc = true;

  // Нет TZID и нет Z — локальное время. Если это чистая дата YYYYMMDD.
  if (info.timezone.empty() && !info.is_utc && info.value.size() == 8 &&
      std::isdigit(static_cast<unsigned char>(info.value[0]))) {
    info.is_date = true;
  }

  return info;
}

datetime_info parse_datetime_field(const std::string& raw_line) {
  auto colon_pos = raw_line.find(':');
  if (colon_pos == std::string::npos)
    return datetime_info{};
  std::string key_part = raw_line.substr(0, colon_pos);
  std::string value_part = raw_line.substr(colon_pos + 1);

  std::string params;
  auto semi_pos = key_part.find(';');
  if (semi_pos != std::string::npos)
    params = key_part.substr(semi_pos + 1);

  return make_datetime_info(params, value_part);
}

std::chrono::time_point<std::chrono::system_clock> parse_ics_datetime(const datetime_info& info,
                                                                      const timezone_resolver& tz) {
  std::tm tm = {};
  std::string val = info.value;

  if (info.is_date && val.size() == 8) {
    // YYYYMMDD
    tm.tm_year = std::stoi(val.substr(0, 4)) - 1900;
    tm.tm_mon = std::stoi(val.substr(4, 2)) - 1;
    tm.tm_mday = std::stoi(val.substr(6, 2));
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
  } else if (val.size() >= 15) {
    // YYYYMMDDTHHMMSS или YYYYMMDDTHHMMSSZ
    tm.tm_year = std::stoi(val.substr(0, 4)) - 1900;
    tm.tm_mon = std::stoi(val.substr(4, 2)) - 1;
    tm.tm_mday = std::stoi(val.substr(6, 2));
    tm.tm_hour = std::stoi(val.substr(9, 2));
    tm.tm_min = std::stoi(val.substr(11, 2));
    tm.tm_sec = std::stoi(val.substr(13, 2));
  }

  tm.tm_isdst = -1;  // не определяем автоматически

  // Преобразуем локальное «стенное» время в абсолютный момент (UTC).
  // Для известных TZID учитываем смещение от UTC, иначе полагаемся на
  // системный часовой пояс (актуально для Europe/Moscow по умолчанию).
  time_t t;
  if (info.is_utc) {
    t = _mkgmtime(&tm);
  } else if (!info.timezone.empty() && !info.is_date) {
    int off = tz.offset_hours(info.timezone);
    if (off != kUnknownOffset) {
      time_t wall = _mkgmtime(&tm);
      t = wall - static_cast<time_t>(off) * 3600;
    } else {
      t = mktime(&tm);
    }
  } else {
    t = mktime(&tm);
  }

  if (t == -1)
    return std::chrono::time_point<std::chrono::system_clock>();

  return std::chrono::system_clock::from_time_t(t);
}

std::chrono::time_point<std::chrono::system_clock> parse_stamp(const std::string& value) {
  if (value.size() < 15)
    return std::chrono::time_point<std::chrono::system_clock>();

  try {
    std::tm tm = {};
    tm.tm_year = std::stoi(value.substr(0, 4)) - 1900;
    tm.tm_mon = std::stoi(value.substr(4, 2)) - 1;
    tm.tm_mday = std::stoi(value.substr(6, 2));
    tm.tm_hour = std::stoi(value.substr(9, 2));
    tm.tm_min = std::stoi(value.substr(11, 2));
    tm.tm_sec = std::stoi(value.substr(13, 2));
    tm.tm_isdst = 0;

    time_t t = _mkgmtime(&tm);
    return t == -1 ? std::chrono::time_point<std::chrono::system_clock>() : std::chrono::system_clock::from_time_t(t);
  } catch (...) {
    return std::chrono::time_point<std::chrono::system_clock>();
  }
}

std::string format_local_dt(std::chrono::time_point<std::chrono::system_clock> tp) {
  auto t = std::chrono::system_clock::to_time_t(tp);
  struct tm tm;
  localtime_s(&tm, &t);
  char buf[20];
  strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", &tm);
  return std::string(buf);
}

}  // namespace divoomdev::ics::detail
