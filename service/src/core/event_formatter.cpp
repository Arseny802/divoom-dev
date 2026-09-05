#include "event_formatter.h"

#include <ctime>
#include <format>

namespace divoomdev::service::core {

event_formatter::event_formatter() noexcept = default;
event_formatter::~event_formatter() noexcept = default;
event_formatter::event_formatter(std::string default_message) noexcept: default_message_(std::move(default_message)) { }

std::string format_local_time(std::chrono::time_point<std::chrono::system_clock> tp, const char* fmt) {
  auto t = std::chrono::system_clock::to_time_t(tp);
  struct tm tm;
  localtime_s(&tm, &t);
  char buf[64];
  strftime(buf, sizeof(buf), fmt, &tm);
  return std::string(buf);
}

std::string event_formatter::format(const event_list& events) {
  if (events.empty()) {
    return default_message_;
  }

  std::string text;
  text.reserve(40 * sizeof(char*) * 512);

  for (const auto& event: events) {
    log()->info("{} {} {} {}",
                event.summary,
                event.location,
                format_local_time(event.start, "%Y-%m-%d %H:%M:%S"),
                format_local_time(event.end, "%Y-%m-%d %H:%M:%S"));

    text += std::format(
        "{}-{}: {}\n", format_local_time(event.start, "%H:%M"), format_local_time(event.end, "%H:%M"), event.summary);
  }

  return text;
}

std::string event_formatter::format(const ics::scheduled_event_list& events) {
  event_list event_list;
  const auto scheduled_event_to_event = [](const ics::scheduled_event& event) -> common::event { return event; };
  std::transform(events.cbegin(), events.cend(), std::back_inserter(event_list), scheduled_event_to_event);
  return format(event_list);
}

}  // namespace divoomdev::service::core
