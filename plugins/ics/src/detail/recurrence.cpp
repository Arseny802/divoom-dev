#include "recurrence.h"

#include "text_utils.h"

#include <ctime>
#include <regex>
#include <sstream>

namespace divoomdev::ics::detail {

rrule_data parse_rrule_data(const std::string& rrule) {
  rrule_data result;
  if (rrule.empty())
    return result;

  // Парсим каждый параметр отдельно, так как порядок может быть любым.
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

bool is_excluded(std::chrono::time_point<std::chrono::system_clock> instance_start,
                 const std::set<std::string>& exdates) {
  if (exdates.empty())
    return false;
  auto t = std::chrono::system_clock::to_time_t(instance_start);
  struct tm tm;
  localtime_s(&tm, &t);
  char date_str[16];
  strftime(date_str, sizeof(date_str), "%Y%m%dT%H%M%S", &tm);
  return exdates.find(date_str) != exdates.end();
}

void apply_repeat_metadata(scheduled_event& ev, const rrule_data& rule) {
  switch (rule.freq) {
  case rrule_data::freq_t::DAILY: ev.repeat = scheduled_event::repeat_t::DAILY; break;
  case rrule_data::freq_t::WEEKLY: ev.repeat = scheduled_event::repeat_t::WEEKLY; break;
  case rrule_data::freq_t::MONTHLY: ev.repeat = scheduled_event::repeat_t::MONTHLY; break;
  case rrule_data::freq_t::YEARLY: ev.repeat = scheduled_event::repeat_t::YEARLY; break;
  }
  ev.recurrence_interval = rule.interval;
  for (int d: rule.byday) {
    ev.byday_mask = static_cast<scheduled_event::day_periodic>(ev.byday_mask | (1 << (d - 1)));
  }
  if (rule.until.time_since_epoch().count() != 0) {
    ev.recurrence_end = rule.until;
  }
}

// ===================== Стратегии раскрытия =====================

namespace {

class noop_expander final : public recurrence_expander {
 public:
  void expand(const scheduled_event&,
              const rrule_data&,
              std::chrono::time_point<std::chrono::system_clock>,
              std::chrono::time_point<std::chrono::system_clock>,
              const std::set<std::string>&,
              std::vector<scheduled_event>&) const override { }
};

class daily_expander final : public recurrence_expander {
 public:
  void expand(const scheduled_event& base,
              const rrule_data& rule,
              std::chrono::time_point<std::chrono::system_clock> window_start,
              std::chrono::time_point<std::chrono::system_clock> deadline,
              const std::set<std::string>& exdates,
              std::vector<scheduled_event>& output) const override {
    auto duration = base.end - base.start;
    for (int i = 0; i < 365; ++i) {  // максимум 365 повторов
      auto instance_start = base.start + std::chrono::hours(24) * i * rule.interval;
      if (instance_start < window_start)
        continue;
      if (instance_start > deadline)
        break;
      if (rule.until.time_since_epoch().count() != 0 && instance_start > rule.until)
        break;
      if (is_excluded(instance_start, exdates))
        continue;

      scheduled_event ev = base;
      ev.start = instance_start;
      ev.end = instance_start + duration;
      output.push_back(std::move(ev));
    }
  }
};

class weekly_expander final : public recurrence_expander {
 public:
  void expand(const scheduled_event& base,
              const rrule_data& rule,
              std::chrono::time_point<std::chrono::system_clock> window_start,
              std::chrono::time_point<std::chrono::system_clock> deadline,
              const std::set<std::string>& exdates,
              std::vector<scheduled_event>& output) const override {
    // Копируем byday: если не указан, используем день недели DTSTART.
    rrule_data mutable_rrule = rule;
    if (mutable_rrule.byday.empty()) {
      auto t = std::chrono::system_clock::to_time_t(base.start);
      struct tm tm;
      localtime_s(&tm, &t);
      int dow = tm.tm_wday;  // 0=Вс, 1=Пн, ..., 6=Сб
      if (dow == 0)
        dow = 7;             // -> 1=Пн, ..., 7=Вс
      mutable_rrule.byday.insert(dow);
    }

    auto duration = base.end - base.start;

    // Опорная точка серии — дата DTSTART события. Повторения следуют от неё
    // каждые INTERVAL недель, поэтому «не та» неделя (например, выпадающая из
    // двухнедельной серии пятница) не генерируется.
    time_t base_t = std::chrono::system_clock::to_time_t(base.start);
    struct tm base_tm;
    localtime_s(&base_tm, &base_t);
    int base_dow = base_tm.tm_wday;
    if (base_dow == 0)
      base_dow = 7;

    for (long long week = 0; week < 520; ++week) {  // до 10 лет серии
      auto week_anchor = base.start + std::chrono::hours(24) * (week * 7LL * rule.interval);
      if (week_anchor > deadline)
        break;

      for (int day: mutable_rrule.byday) {
        int days_ahead = (day - base_dow + 7) % 7;
        long long day_offset = days_ahead + week * 7LL * rule.interval;

        auto instance_start = base.start + std::chrono::hours(24) * day_offset;
        if (instance_start < window_start)
          continue;
        if (instance_start > deadline)
          continue;
        if (rule.until.time_since_epoch().count() != 0 && instance_start > rule.until)
          continue;
        if (is_excluded(instance_start, exdates))
          continue;

        scheduled_event ev = base;
        ev.start = instance_start;
        ev.end = instance_start + duration;
        output.push_back(std::move(ev));
      }
    }
  }
};

class monthly_expander final : public recurrence_expander {
 public:
  void expand(const scheduled_event& base,
              const rrule_data& rule,
              std::chrono::time_point<std::chrono::system_clock> window_start,
              std::chrono::time_point<std::chrono::system_clock> deadline,
              const std::set<std::string>& exdates,
              std::vector<scheduled_event>& output) const override {
    (void)window_start;  // месячная серия ограничивается только дедлайном/UNTIL
    auto duration = base.end - base.start;
    for (int i = 0; i < 24; ++i) {  // максимум 24 месяца
      auto t = std::chrono::system_clock::to_time_t(base.start);
      struct tm tm;
      localtime_s(&tm, &t);

      tm.tm_mon += i * rule.interval;
      tm.tm_hour = 0;
      tm.tm_min = 0;
      tm.tm_sec = 0;

      time_t new_t = mktime(&tm);
      if (new_t == -1)
        continue;

      auto instance_start = std::chrono::system_clock::from_time_t(new_t);
      if (instance_start > deadline)
        break;
      if (rule.until.time_since_epoch().count() != 0 && instance_start > rule.until)
        break;
      if (is_excluded(instance_start, exdates))
        continue;

      scheduled_event ev = base;
      ev.start = instance_start;
      ev.end = instance_start + duration;
      output.push_back(std::move(ev));
    }
  }
};

const daily_expander g_daily;
const weekly_expander g_weekly;
const monthly_expander g_monthly;
const noop_expander g_noop;

}  // namespace

const recurrence_expander& recurrence_expander::for_freq(rrule_data::freq_t freq) {
  switch (freq) {
  case rrule_data::freq_t::DAILY: return g_daily;
  case rrule_data::freq_t::WEEKLY: return g_weekly;
  case rrule_data::freq_t::MONTHLY: return g_monthly;
  case rrule_data::freq_t::YEARLY: return g_noop;
  }
  return g_noop;
}

}  // namespace divoomdev::ics::detail