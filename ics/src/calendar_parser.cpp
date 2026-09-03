#include "calendar_parser.h"

#include <algorithm>
#include <sstream>

#include "cpr/cpr.h"
#include "datetime.h"
#include "event_builder.h"
#include "override_applier.h"
#include "raw_event.h"
#include "recurrence.h"
#include "text_utils.h"
#include "timezone.h"
#include "window.h"

#include "ics/parser.h"

namespace divoomdev::ics::detail {

namespace {

/// Сканирует unfold'нутый контент и собирает сырые события через event_builder.
std::vector<raw_event> collect_raw_events(const std::string& unfolded, const timezone_resolver& tz) {
  std::vector<raw_event> raws;
  event_builder builder(tz);

  std::istringstream ss(unfolded);
  std::string line;
  bool in_event = false;
  bool in_alarm = false;
  std::vector<std::string> event_lines;
  std::vector<std::string> alarm_lines;

  auto flush = [&]() {
    if (!event_lines.empty()) {
      raw_event re = builder.build(event_lines, alarm_lines);
      if (!re.ev.uid.empty())
        raws.push_back(std::move(re));
    }
    event_lines.clear();
    alarm_lines.clear();
  };

  while (std::getline(ss, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    std::string t = trim(line);

    if (t == "BEGIN:VEVENT") {
      in_event = true;
      in_alarm = false;
      event_lines.clear();
      alarm_lines.clear();
      continue;
    }
    if (t == "END:VEVENT") {
      in_event = false;
      in_alarm = false;
      flush();
      continue;
    }
    if (!in_event)
      continue;
    if (t == "BEGIN:VALARM") {
      in_alarm = true;
      alarm_lines.clear();
      continue;
    }
    if (t == "END:VALARM") {
      in_alarm = false;
      continue;
    }
    if (in_alarm)
      alarm_lines.push_back(t);
    else
      event_lines.push_back(t);
  }

  return raws;
}

}  // namespace

std::vector<scheduled_event> parse_calendar_at(const std::string& ics_content,
                                               std::chrono::time_point<std::chrono::system_clock> now) {
  // 1. Собираем TZID-смещения из VTIMEZONE (для Outlook/Exchange Windows-имён поясов).
  timezone_resolver tz;
  tz.collect_vtimezones(ics_content);

  // 2. Разворачиваем long lines (RFC 5545 line folding) за один проход.
  std::string unfolded = unfold(ics_content);

  // 3. Собираем сырые VEVENT.
  std::vector<raw_event> raws = collect_raw_events(unfolded, tz);

  // 4. Окно: от начала сегодняшнего дня до now+48ч.
  time_window win = compute_window(now);

  // 5. Индекс RECURRENCE-ID-оверрайдов по UID.
  auto overrides = override_applier::build_index(raws);

  // 6. Раскрываем мастер-события (пропуская переопределённые вхождения).
  std::vector<scheduled_event> result;
  for (const auto& r: raws) {
    if (r.has_recurrence_id)
      continue;  // оверрайды добавляются отдельно ниже
    if (r.ev.status == common::event_status::CANCELED)
      continue;

    if (r.has_rrule) {
      scheduled_event base = r.ev;
      apply_repeat_metadata(base, r.rrule);

      std::vector<scheduled_event> instances;
      recurrence_expander::for_freq(r.rrule.freq).expand(base, r.rrule, win.start, win.deadline, r.exdates, instances);

      for (auto& inst: instances) {
        if (override_applier::is_overridden(overrides, r.ev.uid, format_local_dt(inst.start)))
          continue;
        result.push_back(std::move(inst));
      }
    } else if (in_window(r.ev.start, win)) {
      result.push_back(r.ev);
    }
  }

  // 7. Добавляем неотменённые оверрайды как самостоятельные события.
  override_applier::add_in_window_overrides(overrides, raws, win, result);

  // 8. Сортируем по времени начала.
  std::sort(result.begin(), result.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  });

  return result;
}

}  // namespace divoomdev::ics::detail

namespace divoomdev::ics {

std::vector<scheduled_event> parse_calendar(const std::string& ics_content) {
  return detail::parse_calendar_at(ics_content, std::chrono::system_clock::now());
}

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

}  // namespace divoomdev::ics