#include "pipeline.h"

#include <algorithm>
#include <sstream>

#include "datetime.h"
#include "event_builder.h"
#include "override_applier.h"
#include "raw_event.h"
#include "recurrence.h"
#include "text_utils.h"
#include "timezone.h"
#include "window.h"

namespace divoomdev::ics::detail {

namespace {

/// Scans unfolded content and collects raw events through the event_builder.
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

scheduled_event_list parse_calendar_at(const std::string& ics_content,
                                       std::chrono::time_point<std::chrono::system_clock> now,
                                       const calendar_settings& settings) {
  // 1. Collect TZID offsets from VTIMEZONE blocks (Outlook/Exchange Windows names).
  timezone_resolver tz;
  tz.collect_vtimezones(ics_content);

  // 2. Unfold long lines (RFC 5545 line folding) in a single pass.
  std::string unfolded = unfold(ics_content);

  // 3. Collect raw VEVENTs.
  std::vector<raw_event> raws = collect_raw_events(unfolded, tz);

  // 4. Build the selection window from the consumer's settings.
  time_window win = compute_window(now, settings);

  // 5. Index RECURRENCE-ID overrides by UID.
  auto overrides = override_applier::build_index(raws);

  // 6. Expand master events, skipping overridden occurrences.
  std::vector<scheduled_event> result;
  for (const auto& r: raws) {
    if (r.has_recurrence_id)
      continue;  // overrides are added separately below
    if (r.ev.status == common::event_status::CANCELED)
      continue;

    if (r.has_rrule) {
      scheduled_event base = r.ev;
      apply_repeat_metadata(base, r.rrule);

      std::vector<scheduled_event> instances;
      recurrence_expander::for_freq(r.rrule.freq)
          .expand(base, r.rrule, win.start, win.deadline, r.exdates, instances);

      for (auto& inst: instances) {
        if (override_applier::is_overridden(overrides, r.ev.uid, format_local_dt(inst.start)))
          continue;
        result.push_back(std::move(inst));
      }
    } else if (in_window(r.ev.start, win)) {
      result.push_back(r.ev);
    }
  }

  // 7. Add non-cancelled overrides as standalone events.
  override_applier::add_in_window_overrides(overrides, raws, win, result);

  // 8. Sort by start time.
  std::sort(result.begin(), result.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  });

  return result;
}

}  // namespace divoomdev::ics::detail