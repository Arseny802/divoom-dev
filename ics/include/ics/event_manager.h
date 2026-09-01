#pragma once
#include "common/event.h"
#include "scheduled_event.h"

namespace divoomdev::ics {

class event_manager {
 public:
  event_manager();
  ~event_manager();

  void add_calendar_url(const std::string& url);
  void add_event(scheduled_event event);

  common::event_list get_today_events();

 private:
  scheduled_event_list events_;
  std::list<std::string> calendar_urls_;
};

}  // namespace divoomdev::ics
