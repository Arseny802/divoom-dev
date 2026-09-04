#include "ics/parser.h"

#include <chrono>

#include "detail/pipeline.h"

namespace divoomdev::ics {

calendar_parser::calendar_parser(calendar_settings settings): settings_(settings) { }

scheduled_event_list calendar_parser::parse(const std::string& ics_content) const {
  return detail::parse_calendar_at(ics_content, std::chrono::system_clock::now(), settings_);
}

scheduled_event_list calendar_parser::parse_at(const std::string& ics_content,
                                               std::chrono::time_point<std::chrono::system_clock> now) const {
  return detail::parse_calendar_at(ics_content, now, settings_);
}

void calendar_parser::set_settings(const calendar_settings& settings) {
  settings_ = settings;
}

const calendar_settings& calendar_parser::settings() const noexcept {
  return settings_;
}

}  // namespace divoomdev::ics