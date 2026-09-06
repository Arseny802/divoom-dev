#pragma once
#include <list>
#include <memory>
#include <string>

#include "ics/cache.h"
#include "ics/scheduled_event.h"
#include "ics/settings.h"
#include "ics/source.h"

namespace divoomdev::ics {

/// Orchestrates calendar loading.
///
/// It fetches raw content through an injected i_calendar_source, caches it in an
/// ics_cache (temporary cache) and exposes the events that fall into the window
/// configured by calendar_settings. All collaborators are injected, which makes
/// the class easy to test and the network layer trivially mockable.
class event_manager {
 public:
  /// Creates a manager wired to the real HTTP source with default settings.
  event_manager();
  ~event_manager();

  /// Creates a manager with explicit collaborators (source is required).
  event_manager(std::shared_ptr<i_calendar_source> source,
                std::shared_ptr<ics_cache> cache = nullptr,
                calendar_settings settings = {});

  void set_source(std::shared_ptr<i_calendar_source> source);
  void set_cache(std::shared_ptr<ics_cache> cache);
  void set_settings(const calendar_settings& settings);

  /// Registers a calendar URL to load when a selection method is called.
  void add_calendar_url(const std::string& url);

  /// Adds an event manually (useful for tests and pre-seeded calendars).
  void add_event(scheduled_event event);

  /// Loads registered URLs (via the cached source) and returns the events within
  /// the configured window. Events marked CANCELED are excluded.
  scheduled_event_list get_next_events();

  /// Convenience wrapper kept for compatibility (upper bound only).
  scheduled_event_list get_next_events_scheduled();

 private:
  void load_from_sources();

  /// Concatenates manual and fetched events (without mutation).
  scheduled_event_list combined() const;

  std::shared_ptr<i_calendar_source> source_;
  std::shared_ptr<ics_cache> cache_;
  calendar_settings settings_;
  scheduled_event_list manual_events_;   // added via add_event()
  scheduled_event_list fetched_events_;  // loaded from calendar_urls_
  std::set<std::string> calendar_urls_;
};

}  // namespace divoomdev::ics
