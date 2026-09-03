#pragma once
#include <chrono>
#include <string>

#include "timezone.h"

namespace divoomdev::ics::detail {

/// Разобранные параметры даты/времени ICS.
/// DTSTART;TZID=Europe/Moscow:20231202T120000 -> timezone="Europe/Moscow", value="20231202T120000"
/// DTSTART:20231202T120000Z                   -> is_utc=true, value="20231202T120000Z"
/// DTSTART;VALUE=DATE:20231202                -> is_date=true, value="20231202"
struct datetime_info {
  std::string timezone;
  bool is_date = false;
  bool is_utc = false;
  std::string value;
};

/// Собирает datetime_info из параметров (params) и значения (value) без ключа.
datetime_info make_datetime_info(const std::string& params, const std::string& value);

/// Парсит строку "KEY[;PARAM=..]:VALUE" в datetime_info.
datetime_info parse_datetime_field(const std::string& raw_line);

/// Преобразует «стенное» локальное время в абсолютный момент (UTC),
/// используя timezone_resolver для известных TZID.
std::chrono::time_point<std::chrono::system_clock> parse_ics_datetime(const datetime_info& info,
                                                                      const timezone_resolver& tz);

/// Парсит CREATED/LAST-MODIFIED/DTSTAMP (всегда UTC).
std::chrono::time_point<std::chrono::system_clock> parse_stamp(const std::string& value);

/// Форматирует time_point в локальную дату/время "%Y%m%dT%H%M%S" (для EXDATE/оверрайдов).
std::string format_local_dt(std::chrono::time_point<std::chrono::system_clock> tp);

}  // namespace divoomdev::ics::detail