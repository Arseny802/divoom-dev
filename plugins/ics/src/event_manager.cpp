#include "ics/event_manager.h"

#include <algorithm>
#include <iterator>

#include "ics/http_source.h"
#include "ics/parser.h"

#include "detail/window.h"

namespace divoomdev::ics {

event_manager::event_manager(): source_(std::make_shared<http_calendar_source>()) {
  AUTOTRACE;
}
event_manager::~event_manager() {
  AUTOTRACE;
}

event_manager::event_manager(std::shared_ptr<i_calendar_source> source,
                             std::shared_ptr<ics_cache> cache,
                             calendar_settings settings)
    : source_(std::move(source)),
      cache_(std::move(cache)),
      settings_(settings) {
  AUTOTRACE;
}

void event_manager::set_source(std::shared_ptr<i_calendar_source> source) {
  source_ = std::move(source);
}

void event_manager::set_cache(std::shared_ptr<ics_cache> cache) {
  cache_ = std::move(cache);
}

void event_manager::set_settings(const calendar_settings& settings) {
  settings_ = settings;
}

void event_manager::add_calendar_url(const std::string& url) {
  calendar_urls_.emplace(url);
}

void event_manager::add_event(scheduled_event event) {
  manual_events_.push_back(std::move(event));
}

void event_manager::load_from_sources() {
  AUTOTRACE;
  if (!source_) {
    hlog()->error("[ICS]: No calendar source configured");
    return;
  }

  // Replace previously fetched events with a fresh load (keeps manual events intact).
  fetched_events_.clear();
  calendar_parser parser(settings_);

  for (const auto& url: calendar_urls_) {
    std::string content;
    bool use_cached = false;

    // Try fresh cache first
    if (cache_ && cache_->get(url, content)) {
      hlog()->info("[ICS]: Cache hit for {}", url);  // no network round-trip
      use_cached = true;
    } else {
      // Fetch from network
      content = source_->fetch(url);
      if (cache_ && !content.empty()) {
        cache_->put(url, content);
      }
    }

    // If network fetch failed and we have stale cached data, use it
    if (content.empty() && cache_ && cache_->get_stale(url, content)) {
      hlog()->warn("[ICS]: Network fetch failed for {}, using stale cached data", url);
      use_cached = true;
    }

    // If still no content, skip this URL (parser handles empty content gracefully)
    if (content.empty()) {
      hlog()->error("[ICS]: No data available for {} (network failed, no cache)", url);
      continue;
    }

    auto parsed = parser.parse(content);
    if (use_cached) {
      hlog()->info("[ICS]: Parsed {} events from {} (cached)", parsed.size(), url);
    } else {
      hlog()->info("[ICS]: Parsed {} events from {}", parsed.size(), url);
    }
    fetched_events_.insert(
        fetched_events_.end(), std::make_move_iterator(parsed.begin()), std::make_move_iterator(parsed.end()));
  }
}

scheduled_event_list event_manager::combined() const {
  scheduled_event_list all = manual_events_;
  all.insert(all.end(), fetched_events_.begin(), fetched_events_.end());
  return all;
}

scheduled_event_list event_manager::get_next_events() {
  AUTOFLUSH;
  AUTOTRACE;
  load_from_sources();
  auto win = detail::compute_window(std::chrono::system_clock::now(), settings_);

  auto all = combined();
  scheduled_event_list selected;
  for (auto& ev: all) {
    if (detail::in_window(ev.start, win))
      selected.push_back(std::move(ev));
  }

  std::sort(selected.begin(), selected.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  });

  hlog()->info("Received {} events from {} sources", selected.size(), calendar_urls_.size());
  return selected;
}

scheduled_event_list event_manager::get_next_events_scheduled() {
  AUTOFLUSH;
  AUTOTRACE;
  load_from_sources();

  auto deadline = std::chrono::system_clock::now() + settings_.horizon;
  auto all = combined();

  scheduled_event_list filtered;
  for (auto& ev: all) {
    if (ev.start <= deadline)
      filtered.push_back(std::move(ev));
  }

  std::sort(filtered.begin(), filtered.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  });

  hlog()->info("Received {} events from {} sources", filtered.size(), calendar_urls_.size());
  return filtered;
}

}  // namespace divoomdev::ics
