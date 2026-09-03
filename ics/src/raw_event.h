#pragma once
#include <set>
#include <string>

#include "ics/scheduled_event.h"
#include "recurrence.h"

namespace divoomdev::ics::detail {

/// Сырое событие до раскрытия повторений и применения RECURRENCE-ID-оверрайдов.
struct raw_event {
  scheduled_event ev;
  rrule_data rrule;
  bool has_rrule = false;
  std::set<std::string> exdates;  // локальные даты/время "%Y%m%dT%H%M%S"
  bool has_recurrence_id = false;
  std::string recurrence_id;      // локальная дата/время исходного вхождения
};

}  // namespace divoomdev::ics::detail