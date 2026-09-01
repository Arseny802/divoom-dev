#include "ics/event_manager.h"

namespace divoomdev::ics {

event_manager::event_manager() { }
event_manager::~event_manager() = default;

void event_manager::add_calendar_url(const std::string& url) {
  calendar_urls_.emplace_back(url);
}
void event_manager::add_event(scheduled_event event) { }

common::event_list event_manager::get_today_events() {
  return {};
}

}  // namespace divoomdev::ics
