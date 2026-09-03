#include "ics/parser.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cpr/cpr.h>
#include <ctime>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace divoomdev::ics {

namespace {

// ===================== Утилиты =====================

/// Развертка длинных строк по RFC 5545 (line folding)
std::string unfold(const std::string& line) {
  std::string result;
  result.reserve(line.size());
  for (size_t i = 0; i < line.size(); ++i) {
    if (line[i] == '\r' && i + 1 < line.size() && line[i + 1] == '\n') {
      i++;       // пропускаем \r
      continue;  // пропускаем \n, это конец строки
    }
    if (line[i] == ' ' && (i == 0 || line[i - 1] == '\n')) {
      continue;  // продолжаем предыдущую строку
    }
    result += line[i];
  }
  return result;
}

/// Снимает quotes и раскодировывает ICS-экранирование
std::string unescape(const std::string& value) {
  std::string result;
  result.reserve(value.size());
  for (size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '\\' && i + 1 < value.size()) {
      switch (value[i + 1]) {
      case 'n':
      case 'N':
        result += '\n';
        i++;
        break;
      case ',':
      case ';':
      case '\\':
        result += value[i + 1];
        i++;
        break;
      default: result += value[i]; break;
      }
    } else {
      result += value[i];
    }
  }
  return result;
}

/// Trim пробелов
std::string trim(const std::string& s) {
  auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos)
    return "";
  auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

/// Приводит строку к верхнему регистру
std::string to_upper(const std::string& s) {
  std::string r = s;
  std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return std::toupper(c); });
  return r;
}

/// Парсит DTSTART/DTEND с параметрами, например:
///   DTSTART;TZID=Europe/Moscow:20231202T120000
///   DTSTART:20231202T120000Z
///   DTSTART;VALUE=DATE:20231202
/// Возвращает (timezone, is_date, is_utc, datetime_str)
struct datetime_info {
  std::string timezone;
  bool is_date = false;
  bool is_utc = false;
  std::string value;
};

datetime_info parse_datetime_field(const std::string& raw_line) {
  datetime_info info;
  // Формат: KEY[;PARAM=value:...]:VALUE
  auto colon_pos = raw_line.find(':');
  if (colon_pos == std::string::npos)
    return info;

  std::string key_part = raw_line.substr(0, colon_pos);
  std::string value_part = raw_line.substr(colon_pos + 1);

  info.value = value_part;

  // Проверяем ключ
  std::string key = to_upper(key_part);

  // Проверяем параметры
  if (key.find(';') != std::string::npos) {
    // Есть параметры
    auto semi_pos = key.find(';');
    std::string params = key.substr(semi_pos + 1);
    std::string base_key = key.substr(0, semi_pos);

    // Ищем TZID
    auto tz_pos = params.find("TZID=");
    if (tz_pos != std::string::npos) {
      auto tz_start = tz_pos + 5;
      auto tz_end = params.find(';', tz_start);
      if (tz_end == std::string::npos)
        tz_end = params.size();
      info.timezone = params.substr(tz_start, tz_end - tz_start);
    }

    // Проверяем VALUE=DATE
    auto val_pos = params.find("VALUE=DATE");
    if (val_pos != std::string::npos) {
      info.is_date = true;
    }

    // Проверяем TZID для определения локального времени
    if (!info.timezone.empty()) {
      info.is_utc = false;
    }
  }

  // Определяем, UTC ли это
  if (!info.value.empty() && info.value.back() == 'Z') {
    info.is_utc = true;
  }

  // Если нет TZID и нет Z — это локальное время (без часового пояса)
  if (info.timezone.empty() && !info.is_utc && info.value.size() >= 8) {
    // Проверяем, является ли значение датой (YYYYMMDD)
    if (info.value.size() == 8 && std::isdigit(info.value[0])) {
      info.is_date = true;
    }
  }

  return info;
}

// Карта TZID -> смещение от UTC в часах, собранная из блоков VTIMEZONE календаря.
std::map<std::string, int> g_tzid_offsets;

/// Парсит значение TZOFFSETTO вида "+0300"/"-0700"/"+0530" в часы (со знаком).
int parse_tzoffset(const std::string& s) {
  if (s.size() < 5)
    return INT_MIN;
  int sign = 1;
  size_t i = 0;
  if (s[0] == '+') {
    i = 1;
  } else if (s[0] == '-') {
    sign = -1;
    i = 1;
  }
  if (i + 4 > s.size())
    return INT_MIN;
  try {
    int hh = std::stoi(s.substr(i, 2));
    int mm = std::stoi(s.substr(i + 2, 2));
    int hours = hh + (mm >= 30 ? 1 : 0);  // округляем получасовые смещения
    return sign * hours;
  } catch (...) {
    return INT_MIN;
  }
}

/// Предварительно проходит по VTIMEZONE и запоминает TZID -> смещение.
/// Это нужно для Outlook/Exchange с Windows-именами поясов (Russian Standard Time и т.п.).
void collect_tzid_offsets(const std::string& ics_content) {
  g_tzid_offsets.clear();
  std::istringstream stream(ics_content);
  std::string line;
  bool in_tz = false;
  bool in_std = false;
  std::string cur_tzid;
  int std_offset = INT_MIN;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    std::string t = trim(line);
    if (t.rfind("BEGIN:VTIMEZONE", 0) == 0) {
      in_tz = true;
      in_std = false;
      cur_tzid.clear();
      std_offset = INT_MIN;
      continue;
    }
    if (!in_tz)
      continue;
    if (t.rfind("END:VTIMEZONE", 0) == 0) {
      if (!cur_tzid.empty() && std_offset != INT_MIN)
        g_tzid_offsets[cur_tzid] = std_offset;
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

/// Форматирует time_point в локальную дату/время "%Y%m%dT%H%M%S" (для EXDATE/оверрайдов).
std::string fmt_local_dt(std::chrono::time_point<std::chrono::system_clock> tp) {
  auto t = std::chrono::system_clock::to_time_t(tp);
  struct tm tm;
  localtime_s(&tm, &t);
  char buf[20];
  strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", &tm);
  return std::string(buf);
}

/// Возвращает смещение TZID относительно UTC в часах
/// или INT_MIN, если пояс неизвестен (тогда используем системное локальное время).
int utc_offset_hours(const std::string& tz) {
  // Сначала собранная из VTIMEZONE карта (Windows-имена Outlook и пр.).
  auto it = g_tzid_offsets.find(tz);
  if (it != g_tzid_offsets.end())
    return it->second;

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
  return INT_MIN;
}

/// Парсит строку ICS datetime в time_point
std::chrono::time_point<std::chrono::system_clock> parse_ics_datetime(const datetime_info& info) {

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
    int off = utc_offset_hours(info.timezone);
    if (off != INT_MIN) {
      time_t wall = _mkgmtime(&tm);
      t = wall - static_cast<time_t>(off) * 3600;
    } else {
      t = mktime(&tm);
    }
  } else {
    t = mktime(&tm);
  }

  if (t == -1) {
    return std::chrono::time_point<std::chrono::system_clock>();
  }

  return std::chrono::system_clock::from_time_t(t);
}

/// Парсит CREATED/LAST-MODIFIED/DTSTAMP (всегда UTC)
std::chrono::time_point<std::chrono::system_clock> parse_stamp(const std::string& value) {
  if (value.size() < 15) {
    return std::chrono::time_point<std::chrono::system_clock>();
  }

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

/// Структура для хранения RRULE
struct rrule_data {
  enum class freq_t { DAILY, WEEKLY, MONTHLY, YEARLY };
  freq_t freq = freq_t::DAILY;
  int interval = 1;
  std::chrono::time_point<std::chrono::system_clock> until;  // epoch = без ограничения
  std::set<int> byday;                                       // дни недели (1=Пн, 7=Вс)
};

/// Парсит RRULE и возвращает структуру
rrule_data parse_rrule_data(const std::string& rrule) {
  rrule_data result;
  if (rrule.empty())
    return result;

  // Парсим каждый параметр отдельно, так как порядок может быть любым
  std::regex freq_re("FREQ=(YEARLY|MONTHLY|WEEKLY|DAILY)");
  std::regex interval_re("INTERVAL=([0-9]+)");
  std::regex until_re("UNTIL=([0-9]+T[0-9]+Z?)");
  std::regex byday_re("BYDAY=([A-Z0-9,]+)");

  std::smatch match;

  if (std::regex_search(rrule, match, freq_re)) {
    std::string freq = to_upper(match[1].str());
    if (freq == "DAILY")
      result.freq = rrule_data::freq_t::DAILY;
    else if (freq == "WEEKLY")
      result.freq = rrule_data::freq_t::WEEKLY;
    else if (freq == "MONTHLY")
      result.freq = rrule_data::freq_t::MONTHLY;
    else if (freq == "YEARLY")
      result.freq = rrule_data::freq_t::YEARLY;
  }

  if (std::regex_search(rrule, match, interval_re)) {
    result.interval = std::stoi(match[1].str());
  }

  if (std::regex_search(rrule, match, until_re)) {
    std::string until_str = match[1].str();
    // Удаляем Z в конце, если есть
    if (!until_str.empty() && until_str.back() == 'Z')
      until_str.pop_back();

    std::tm tm = {};
    tm.tm_year = std::stoi(until_str.substr(0, 4)) - 1900;
    tm.tm_mon = std::stoi(until_str.substr(4, 2)) - 1;
    tm.tm_mday = std::stoi(until_str.substr(6, 2));
    tm.tm_hour = std::stoi(until_str.substr(9, 2));
    tm.tm_min = std::stoi(until_str.substr(11, 2));
    tm.tm_sec = std::stoi(until_str.substr(13, 2));
    tm.tm_isdst = 0;
    time_t t = _mkgmtime(&tm);
    if (t != -1)
      result.until = std::chrono::system_clock::from_time_t(t);
  }

  if (std::regex_search(rrule, match, byday_re)) {
    std::string byday = match[1].str();
    std::istringstream stream(byday);
    std::string day;
    while (std::getline(stream, day, ',')) {
      auto d = to_upper(day);
      if (d == "MO")
        result.byday.insert(1);
      else if (d == "TU")
        result.byday.insert(2);
      else if (d == "WE")
        result.byday.insert(3);
      else if (d == "TH")
        result.byday.insert(4);
      else if (d == "FR")
        result.byday.insert(5);
      else if (d == "SA")
        result.byday.insert(6);
      else if (d == "SU")
        result.byday.insert(7);
    }
  }

  return result;
}

/// Генерирует повторяющиеся события на основе RRULE
void expand_recurrence(const scheduled_event& base_ev,
                       const rrule_data& rrule,
                       const std::chrono::time_point<std::chrono::system_clock>& now,
                       const std::chrono::time_point<std::chrono::system_clock>& deadline,
                       const std::set<std::string>& exdates,
                       std::vector<scheduled_event>& output) {

  // Создаём копию rrule для модификации byday
  rrule_data mutable_rrule = rrule;

  if (rrule.freq == rrule_data::freq_t::DAILY) {
    // Ежедневно
    for (int i = 0; i < 365; ++i) {  // максимум 365 повторов
      auto instance_start = base_ev.start;
      auto instance_end = base_ev.end;

      // Добавляем i интервалов
      auto duration = instance_end - instance_start;
      instance_start += std::chrono::hours(24) * i * rrule.interval;
      instance_end = instance_start + duration;

      // Проверяем, что событие не в прошлом
      if (instance_start < now)
        continue;

      // Проверяем deadline
      if (instance_start > deadline)
        break;

      // Проверяем UNTIL
      if (rrule.until.time_since_epoch().count() != 0 && instance_start > rrule.until)
        break;

      // Формируем строку для проверки EXDATE
      auto t = std::chrono::system_clock::to_time_t(instance_start);
      struct tm tm;
      localtime_s(&tm, &t);
      char date_str[16];
      strftime(date_str, sizeof(date_str), "%Y%m%dT%H%M%S", &tm);
      std::string date_str_std(date_str);

      // Проверяем EXDATE
      if (exdates.find(date_str_std) != exdates.end())
        continue;

      // Создаём копию события
      scheduled_event ev = base_ev;
      ev.start = instance_start;
      ev.end = instance_end;
      output.push_back(std::move(ev));
    }
  } else if (rrule.freq == rrule_data::freq_t::WEEKLY) {
    // Еженедельно по определённым дням
    if (mutable_rrule.byday.empty()) {
      // Если BYDAY не указан, используем день недели DTSTART
      auto t = std::chrono::system_clock::to_time_t(base_ev.start);
      struct tm tm;
      localtime_s(&tm, &t);
      int dow = tm.tm_wday;  // 0=Вс, 1=Пн, ..., 6=Сб
      if (dow == 0)
        dow = 7;             // Преобразуем в 1=Пн, ..., 7=Вс
      mutable_rrule.byday.insert(dow);
    }

    // Вычисляем длительность события
    auto duration = base_ev.end - base_ev.start;

    // Опорная точка серии — дата DTSTART события. Повторения следуют от неё
    // каждые INTERVAL недель, поэтому «не та» неделя (например, выпадающая из
    // двухнедельной серии пятница) не генерируется.
    time_t base_t = std::chrono::system_clock::to_time_t(base_ev.start);
    struct tm base_tm;
    localtime_s(&base_tm, &base_t);
    int base_dow = base_tm.tm_wday;
    if (base_dow == 0)
      base_dow = 7;

    // Перебираем недели серии. week_anchor — самый ранний возможный момент
    // в данной неделе (days_ahead >= 0), поэтому если он уже за дедлайном,
    // дальше искать нечего. Внутри недели каждый день проверяется отдельно,
    // чтобы не терять дни из окна, идущие в byday позже первого.
    for (long long week = 0; week < 520; ++week) {  // до 10 лет серии
      auto week_anchor = base_ev.start + std::chrono::hours(24) * (week * 7LL * rrule.interval);
      if (week_anchor > deadline)
        break;

      for (int day: mutable_rrule.byday) {
        // Первое вхождение дня 'day' начиная с DTSTART
        int days_ahead = (day - base_dow + 7) % 7;
        long long day_offset = days_ahead + week * 7LL * rrule.interval;

        auto instance_start = base_ev.start + std::chrono::hours(24) * day_offset;
        auto instance_end = instance_start + duration;

        // Проверяем нижнюю границу окна (начало текущего дня)
        if (instance_start < now)
          continue;

        // Проверяем deadline
        if (instance_start > deadline)
          continue;

        // Проверяем UNTIL
        if (rrule.until.time_since_epoch().count() != 0 && instance_start > rrule.until)
          continue;

        // Формируем строку для проверки EXDATE (локальное время)
        auto it = std::chrono::system_clock::to_time_t(instance_start);
        struct tm itm;
        localtime_s(&itm, &it);
        char date_str[16];
        strftime(date_str, sizeof(date_str), "%Y%m%dT%H%M%S", &itm);
        std::string date_str_std(date_str);

        // Проверяем EXDATE
        if (exdates.find(date_str_std) != exdates.end())
          continue;

        // Создаём копию события
        scheduled_event ev = base_ev;
        ev.start = instance_start;
        ev.end = instance_end;
        output.push_back(std::move(ev));
      }
    }
  } else if (rrule.freq == rrule_data::freq_t::MONTHLY) {
    // Ежемесячно
    for (int i = 0; i < 24; ++i) {  // максимум 24 месяца
      auto instance_start = base_ev.start;
      auto instance_end = base_ev.end;

      // Добавляем i месяцев
      auto duration = instance_end - instance_start;
      auto t = std::chrono::system_clock::to_time_t(instance_start);
      struct tm tm;
      localtime_s(&tm, &t);

      // Добавляем месяцы
      tm.tm_mon += i * rrule.interval;
      // Сбрасываем часы/минуты/секунды
      tm.tm_hour = 0;
      tm.tm_min = 0;
      tm.tm_sec = 0;

      time_t new_t = mktime(&tm);
      if (new_t == -1)
        continue;

      instance_start = std::chrono::system_clock::from_time_t(new_t);
      instance_end = instance_start + duration;

      // Проверяем deadline
      if (instance_start > deadline)
        break;

      // Проверяем UNTIL
      if (rrule.until.time_since_epoch().count() != 0 && instance_start > rrule.until)
        break;

      // Формируем строку для проверки EXDATE
      auto t2 = std::chrono::system_clock::to_time_t(instance_start);
      struct tm tm2;
      localtime_s(&tm2, &t2);
      char date_str[16];
      strftime(date_str, sizeof(date_str), "%Y%m%dT%H%M%S", &tm2);
      std::string date_str_std(date_str);

      // Проверяем EXDATE
      if (exdates.find(date_str_std) != exdates.end())
        continue;

      // Создаём копию события
      scheduled_event ev = base_ev;
      ev.start = instance_start;
      ev.end = instance_end;
      output.push_back(std::move(ev));
    }
  }
}

/// Парсит VALARM и добавляет alarm в событие
void parse_alarm(const std::vector<std::string>& lines, scheduled_event& ev) {
  std::string trigger_str;
  std::string action = "DISPLAY";
  std::string description;

  for (const auto& line: lines) {
    auto trimmed = trim(line);
    if (trimmed.empty())
      continue;

    auto colon_pos = trimmed.find(':');
    if (colon_pos == std::string::npos)
      continue;

    std::string key_part = trim(trimmed.substr(0, colon_pos));
    std::string value = unescape(trim(trimmed.substr(colon_pos + 1)));

    std::string key = to_upper(key_part);

    if (key == "TRIGGER") {
      trigger_str = value;
    } else if (key == "ACTION") {
      action = value;
    } else if (key == "DESCRIPTION") {
      description = value;
    }
  }

  if (!trigger_str.empty()) {
    common::alarm al;
    al.action = action;
    al.description = description;

    // Парсим триггер: -P1D (за 1 день), -PT2H (за 2 часа), -PT30M (за 30 минут)
    if (trigger_str[0] == '-')
      trigger_str = trigger_str.substr(1);

    if (trigger_str[0] == 'P') {
      std::string duration = trigger_str.substr(1);
      int days = 0, hours = 0, minutes = 0;

      // Парсим дни
      auto d_pos = duration.find('D');
      if (d_pos != std::string::npos) {
        days = std::stoi(duration.substr(0, d_pos));
        duration = duration.substr(d_pos + 1);
      }

      // Парсим время после T
      auto t_pos = duration.find('T');
      if (t_pos != std::string::npos) {
        std::string time_part = duration.substr(t_pos + 1);
        auto h_pos = time_part.find('H');
        if (h_pos != std::string::npos) {
          hours = std::stoi(time_part.substr(0, h_pos));
          time_part = time_part.substr(h_pos + 1);
        }
        auto m_pos = time_part.find('M');
        if (m_pos != std::string::npos) {
          minutes = std::stoi(time_part.substr(0, m_pos));
        }
      } else {
        // Может быть P1D или просто число минут (редко)
        if (!duration.empty() && std::isdigit(duration[0])) {
          minutes = std::stoi(duration);
        }
      }

      al.trigger_minutes = std::chrono::minutes(days * 1440 + hours * 60 + minutes);
    }

    ev.alarms.push_back(al);
  }
}

// ===================== Основной парсер =====================

/// Разделяет строку на ключ и значение, учитывая параметры ключа
/// Например: "DTSTART;TZID=Europe/Moscow:20231202T120000"
/// Возвращает {"DTSTART", "TZID=Europe/Moscow", "20231202T120000"}
std::tuple<std::string, std::string, std::string> split_ics_line(const std::string& line) {
  std::string key, params, value;

  auto colon_pos = line.find(':');
  if (colon_pos == std::string::npos) {
    return {"", "", ""};
  }

  std::string left = line.substr(0, colon_pos);
  value = line.substr(colon_pos + 1);

  auto semi_pos = left.find(';');
  if (semi_pos != std::string::npos) {
    key = left.substr(0, semi_pos);
    params = left.substr(semi_pos + 1);
  } else {
    key = left;
    params = "";
  }

  return {to_upper(key), params, value};
}

/// Сырое событие до раскрытия повторений и применения RECURRENCE-ID-оверрайдов.
struct raw_event {
  scheduled_event ev;
  rrule_data rrule;
  bool has_rrule = false;
  std::set<std::string> exdates;  // локальные даты/время "%Y%m%dT%H%M%S"
  bool has_recurrence_id = false;
  std::string recurrence_id;      // локальная дата/время исходного вхождения
};

}  // namespace

// ===================== API =====================

std::vector<scheduled_event> fetch_events(const std::string& url) {
  cpr::Parameters parameters;
  cpr::SslOptions ssl_options;
  ssl_options.verify_host = false;
  ssl_options.verify_peer = false;
  cpr::Response response = cpr::Get(cpr::Url{url}, parameters, ssl_options);

  hlog()->info("[ICS]: Fetched {}, status={}, size={}", url, response.status_code, response.text.size());

  if (response.status_code != 200) {
    hlog()->error(
        "[ICS]: Failed fetching {} with code={}, error={}", url, response.status_code, response.error.message);
    return {};
  }

  auto events = parse_calendar(response.text);
  hlog()->info("[ICS]: Parsed {} events from calendar", events.size());
  return events;
}

std::vector<scheduled_event> parse_calendar(const std::string& ics_content) {
  // Сначала собираем сырые VEVENT, затем раскрываем повторения с учётом
  // RECURRENCE-ID-оверрайдов (перенесённые/отменённые вхождения).
  std::vector<raw_event> raws;
  collect_tzid_offsets(ics_content);

  // Разворачиваем long lines (RFC 5545 line folding) за один проход.
  // Строка, начинающаяся с пробела/таба — продолжение предыдущей логической
  // строки; обычная строка — новая логическая строка.
  std::string lines;
  std::istringstream stream(ics_content);
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.empty()) {
      lines += "\n";
      continue;
    }
    if (line[0] == ' ' || line[0] == '\t') {
      // Продолжение предыдущей строки: убираем перевод строки и ведущий пробел.
      if (!lines.empty() && lines.back() == '\n')
        lines.pop_back();
      lines += line.substr(1);
    } else {
      // Новая логическая строка.
      if (!lines.empty() && lines.back() != '\n')
        lines += '\n';
      lines += line + "\n";
    }
  }

  // 3. Ищем VEVENT блоки
  std::istringstream final_stream(lines);
  std::string fl;
  bool in_event = false;
  bool in_alarm = false;
  std::vector<std::string> event_lines;
  std::vector<std::string> alarm_lines;

  auto process_event = [&]() {
    scheduled_event ev;
    raw_event re;

    for (const auto& l: event_lines) {
      auto trimmed = trim(l);
      if (trimmed.empty())
        continue;

      auto [key, params, value] = split_ics_line(trimmed);
      if (key.empty())
        continue;

      if (key == "SUMMARY") {
        ev.summary = trim(unescape(value));
      } else if (key == "DESCRIPTION") {
        ev.description = unescape(value);
      } else if (key == "UID") {
        ev.uid = value;
      } else if (key == "LOCATION") {
        ev.location = unescape(value);
      } else if (key == "SEQUENCE") {
        try {
          ev.sequence = std::stoi(value);
        } catch (...) { }
      } else if (key == "URL") {
        ev.url = value;
      } else if (key == "CATEGORIES") {
        ev.categories = unescape(value);
      } else if (key == "STATUS") {
        std::string status_upper = to_upper(value);
        if (status_upper == "CANCELED") {
          ev.status = common::event_status::CANCELED;
        } else if (status_upper == "TENTATIVE") {
          ev.status = common::event_status::TENTATIVE;
        } else {
          ev.status = common::event_status::CONFIRMED;
        }
      } else if (key == "DTSTART") {
        auto dt_info = parse_datetime_field(key + ":" + value);
        // Перепарсим с params
        dt_info.timezone = params;
        // Извлекаем TZID из params
        auto tz_pos = params.find("TZID=");
        if (tz_pos != std::string::npos) {
          auto tz_start = tz_pos + 5;
          auto tz_end = params.find(';', tz_start);
          if (tz_end == std::string::npos)
            tz_end = params.size();
          dt_info.timezone = params.substr(tz_start, tz_end - tz_start);
        }
        ev.start = parse_ics_datetime(dt_info);
      } else if (key == "DTEND") {
        auto dt_info = parse_datetime_field(key + ":" + value);
        auto tz_pos = params.find("TZID=");
        if (tz_pos != std::string::npos) {
          auto tz_start = tz_pos + 5;
          auto tz_end = params.find(';', tz_start);
          if (tz_end == std::string::npos)
            tz_end = params.size();
          dt_info.timezone = params.substr(tz_start, tz_end - tz_start);
        }
        ev.end = parse_ics_datetime(dt_info);
      } else if (key == "CREATED") {
        ev.created = parse_stamp(value);
      } else if (key == "LAST-MODIFIED") {
        ev.last_modified = parse_stamp(value);
      } else if (key == "DTSTAMP") {
        ev.stamp = parse_stamp(value);
      } else if (key == "ORGANIZER") {
        // ORGANIZER;CN=Name:mailto:email
        auto mail_pos = value.find("mailto:");
        if (mail_pos != std::string::npos) {
          ev.organizer = value.substr(mail_pos + 7);
        } else {
          ev.organizer = value;
        }
        // Ищем CN в params
        auto cn_pos = params.find("CN=");
        if (cn_pos != std::string::npos) {
          auto cn_start = cn_pos + 3;
          auto cn_end = params.find(';', cn_start);
          if (cn_end == std::string::npos)
            cn_end = params.size();
          ev.organizer_name = unescape(params.substr(cn_start, cn_end - cn_start));
        }
      } else if (key == "RRULE") {
        re.rrule = parse_rrule_data(value);
        re.has_rrule = true;
      } else if (key == "RECURRENCE-ID") {
        re.has_recurrence_id = true;
        // Локальная дата/время исходного вхождения (после последнего ':').
        auto colon_pos = value.rfind(':');
        re.recurrence_id = colon_pos == std::string::npos ? value : value.substr(colon_pos + 1);
      } else if (key == "EXDATE") {
        // Может быть несколько значений через запятую: A,B,C
        std::istringstream exs(value);
        std::string part;
        while (std::getline(exs, part, ',')) {
          std::string p = trim(part);
          if (!p.empty()) {
            re.exdates.insert(p);
            ev.exdates.insert(p);
          }
        }
      }
    }

    re.ev = std::move(ev);
    if (!re.ev.uid.empty())
      raws.push_back(std::move(re));
  };

  while (std::getline(final_stream, fl)) {
    // Убираем \r
    if (!fl.empty() && fl.back() == '\r')
      fl.pop_back();

    std::string trimmed = trim(fl);
    if (trimmed == "BEGIN:VEVENT") {
      in_event = true;
      event_lines.clear();
      continue;
    } else if (trimmed == "END:VEVENT") {
      in_event = false;
      process_event();
      continue;
    } else if (trimmed == "BEGIN:VALARM") {
      in_alarm = true;
      alarm_lines.clear();
      continue;
    } else if (trimmed == "END:VALARM") {
      in_alarm = false;
      continue;
    }

    if (in_event) {
      event_lines.push_back(trimmed);
    } else if (in_alarm) {
      alarm_lines.push_back(trimmed);
    }
  }

  // 4. Окно: от начала сегодняшнего дня до now+48ч (включая уже начавшиеся сегодня).
  auto now = std::chrono::system_clock::now();
  auto deadline = now + std::chrono::hours(48);

  time_t now_t = std::chrono::system_clock::to_time_t(now);
  struct tm now_tm;
  localtime_s(&now_tm, &now_t);
  now_tm.tm_hour = 0;
  now_tm.tm_min = 0;
  now_tm.tm_sec = 0;
  time_t day_start_t = mktime(&now_tm);
  auto window_start = day_start_t == -1 ? now : std::chrono::system_clock::from_time_t(day_start_t);

  // Индекс RECURRENCE-ID-оверрайдов по UID.
  std::map<std::string, std::vector<const raw_event*>> overrides_by_uid;
  for (const auto& r: raws) {
    if (r.has_recurrence_id)
      overrides_by_uid[r.ev.uid].push_back(&r);
  }

  std::vector<scheduled_event> result;
  for (const auto& r: raws) {
    if (r.has_recurrence_id)
      continue;  // оверрайды добавляются отдельно ниже
    if (r.ev.status == common::event_status::CANCELED)
      continue;

    if (r.has_rrule) {
      scheduled_event base = r.ev;
      switch (r.rrule.freq) {
      case rrule_data::freq_t::DAILY: base.repeat = scheduled_event::repeat_t::DAILY; break;
      case rrule_data::freq_t::WEEKLY: base.repeat = scheduled_event::repeat_t::WEEKLY; break;
      case rrule_data::freq_t::MONTHLY: base.repeat = scheduled_event::repeat_t::MONTHLY; break;
      case rrule_data::freq_t::YEARLY: base.repeat = scheduled_event::repeat_t::YEARLY; break;
      }
      base.recurrence_interval = r.rrule.interval;
      for (int d: r.rrule.byday) {
        base.byday_mask = static_cast<scheduled_event::day_periodic>(base.byday_mask | (1 << (d - 1)));
      }
      if (r.rrule.until.time_since_epoch().count() != 0) {
        base.recurrence_end = r.rrule.until;
      }

      std::vector<scheduled_event> instances;
      expand_recurrence(base, r.rrule, window_start, deadline, r.exdates, instances);
      for (auto& inst: instances) {
        // Если на это вхождение есть оверрайд — пропускаем (добавится ниже).
        std::string key = fmt_local_dt(inst.start);
        bool overridden = false;
        auto it = overrides_by_uid.find(r.ev.uid);
        if (it != overrides_by_uid.end()) {
          for (const auto* ov: it->second) {
            if (ov->recurrence_id == key) {
              overridden = true;
              break;
            }
          }
        }
        if (overridden)
          continue;
        result.push_back(std::move(inst));
      }
    } else if (r.ev.start.time_since_epoch().count() != 0 && r.ev.start >= window_start && r.ev.start <= deadline) {
      result.push_back(r.ev);
    }
  }

  // 5. Добавляем неотменённые оверрайды как самостоятельные события.
  for (const auto& r: raws) {
    if (!r.has_recurrence_id || r.ev.status == common::event_status::CANCELED)
      continue;
    if (r.ev.start.time_since_epoch().count() != 0 && r.ev.start >= window_start && r.ev.start <= deadline) {
      result.push_back(r.ev);
    }
  }

  // 6. Сортируем по времени начала.
  std::sort(result.begin(), result.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  });

  return result;
}

}  // namespace divoomdev::ics
