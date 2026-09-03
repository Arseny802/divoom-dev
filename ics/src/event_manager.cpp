#include "ics/event_manager.h"
#include "ics/parser.h"

namespace divoomdev::ics {

event_manager::event_manager() { }
event_manager::~event_manager() = default;

void event_manager::add_calendar_url(const std::string& url) {
  calendar_urls_.emplace_back(url);
}

void event_manager::add_event(scheduled_event event) {
  events_.push_back(std::move(event));
}

common::event_list event_manager::get_next_48h_events() {
  // Загружаем все календари
  hlog()->info("[ICS]: Loading {} calendar URLs", calendar_urls_.size());
  for (const auto& url: calendar_urls_) {
    hlog()->info("[ICS]: Fetching {}", url);
    auto fetched = fetch_events(url);
    hlog()->info("[ICS]: Fetched {} events, adding to list", fetched.size());
    events_.insert(events_.end(), std::make_move_iterator(fetched.begin()), std::make_move_iterator(fetched.end()));
  }

  hlog()->info("[ICS]: Total events in cache: {}", events_.size());

  // Фильтруем на 48 часов
  auto now = std::chrono::system_clock::now();
  auto deadline = now + std::chrono::hours(48);

  scheduled_event_list filtered;
  for (auto& ev: events_) {
    if (ev.start <= deadline && ev.start >= now) {
      filtered.push_back(std::move(ev));
    }
  }

  hlog()->info("[ICS]: Filtered to {} events in 48h window", filtered.size());

  // Сортируем и возвращаем
  std::sort(filtered.begin(), filtered.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  });

  return common::event_list(filtered.begin(), filtered.end());
}

scheduled_event_list event_manager::get_next_48h_scheduled() {
  // Загружаем все календари
  for (const auto& url: calendar_urls_) {
    auto fetched = fetch_events(url);
    events_.insert(events_.end(), std::make_move_iterator(fetched.begin()), std::make_move_iterator(fetched.end()));
  }

  // Фильтруем на 48 часов
  auto now = std::chrono::system_clock::now();
  auto deadline = now + std::chrono::hours(48);

  scheduled_event_list filtered;
  for (auto& ev: events_) {
    if (ev.start <= deadline) {
      filtered.push_back(std::move(ev));
    }
  }

  // Сортируем
  std::sort(filtered.begin(), filtered.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  });

  return filtered;
}

}  // namespace divoomdev::ics
