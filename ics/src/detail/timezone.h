#pragma once
#include <climits>
#include <map>
#include <string>

namespace divoomdev::ics::detail {

/// Возвращается, когда смещение для пояса неизвестно (тогда полагаемся на системное время).
constexpr int kUnknownOffset = INT_MIN;

/// Парсит значение TZOFFSETTO вида "+0300"/"-0700"/"+0530" в часы (со знаком).
int parse_tzoffset(const std::string& s);

/// Резолвер смещения часового пояса относительно UTC.
///
/// Это реализация паттерна "Стратегия": выбор источника знания о поясе
/// (VTIMEZONE из календаря, статическая таблица Windows/IANA имён или системный
/// часовой пояс) инкапсулирован внутри класса и может быть легко заменён.
class timezone_resolver {
 public:
  /// Предварительно проходит по VTIMEZONE блокам и запоминает TZID -> смещение.
  /// Нужно для Outlook/Exchange с Windows-именами поясов (Russian Standard Time и т.п.).
  void collect_vtimezones(const std::string& ics_content);

  /// Возвращает смещение TZID относительно UTC в часах
  /// или kUnknownOffset, если пояс неизвестен.
  int offset_hours(const std::string& tz) const;

 private:
  std::map<std::string, int> vtz_offsets_;

  /// Статическая запасная таблица известных поясов.
  static int static_offset(const std::string& tz);
};

}  // namespace divoomdev::ics::detail