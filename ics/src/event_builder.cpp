#include "event_builder.h"

#include "datetime.h"
#include "text_utils.h"

#include <cctype>
#include <functional>
#include <map>
#include <sstream>

namespace divoomdev::ics::detail {

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

  if (trigger_str.empty())
    return;

  common::alarm al;
  al.action = action;
  al.description = description;

  // Парсим триггер: -P1D (за 1 день), -PT2H (за 2 часа), -PT30M (за 30 минут).
  if (trigger_str[0] == '-')
    trigger_str = trigger_str.substr(1);

  if (trigger_str[0] == 'P') {
    std::string duration = trigger_str.substr(1);
    int days = 0, hours = 0, minutes = 0;

    auto d_pos = duration.find('D');
    if (d_pos != std::string::npos) {
      days = std::stoi(duration.substr(0, d_pos));
      duration = duration.substr(d_pos + 1);
    }

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
    } else if (!duration.empty() && std::isdigit(static_cast<unsigned char>(duration[0]))) {
      minutes = std::stoi(duration);
    }

    al.trigger_minutes = std::chrono::minutes(days * 1440 + hours * 60 + minutes);
  }

  ev.alarms.push_back(al);
}

namespace {

// Выделяет из параметров "CN=Name;..." значение CN.
std::string param_value(const std::string& params, const std::string& name) {
  auto pos = params.find(name + "=");
  if (pos == std::string::npos)
    return "";
  auto start = pos + name.size() + 1;
  auto end = params.find(';', start);
  if (end == std::string::npos)
    end = params.size();
  return params.substr(start, end - start);
}

}  // namespace

event_builder::event_builder(const timezone_resolver& tz): tz_(tz) { }

raw_event event_builder::build(const std::vector<std::string>& content_lines,
                               const std::vector<std::string>& alarm_lines) const {
  raw_event re;
  scheduled_event& ev = re.ev;

  for (const auto& l: content_lines) {
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
      if (status_upper == "CANCELED")
        ev.status = common::event_status::CANCELED;
      else if (status_upper == "TENTATIVE")
        ev.status = common::event_status::TENTATIVE;
      else
        ev.status = common::event_status::CONFIRMED;
    } else if (key == "DTSTART") {
      ev.start = parse_ics_datetime(make_datetime_info(params, value), tz_);
    } else if (key == "DTEND") {
      ev.end = parse_ics_datetime(make_datetime_info(params, value), tz_);
    } else if (key == "CREATED") {
      ev.created = parse_stamp(value);
    } else if (key == "LAST-MODIFIED") {
      ev.last_modified = parse_stamp(value);
    } else if (key == "DTSTAMP") {
      ev.stamp = parse_stamp(value);
    } else if (key == "ORGANIZER") {
      // ORGANIZER;CN=Name:mailto:email
      auto mail_pos = value.find("mailto:");
      if (mail_pos != std::string::npos)
        ev.organizer = value.substr(mail_pos + 7);
      else
        ev.organizer = value;
      ev.organizer_name = unescape(param_value(params, "CN"));
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

  // Присоединяем alarms из вложенных VALARM-блоков.
  parse_alarm(alarm_lines, ev);

  return re;
}

}  // namespace divoomdev::ics::detail
