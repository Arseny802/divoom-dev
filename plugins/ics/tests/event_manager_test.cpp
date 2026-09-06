#include "gtest/gtest.h"

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "common/event.h"
#include "ics/cache.h"
#include "ics/event_manager.h"
#include "ics/settings.h"
#include "ics/source.h"
#include "test_helpers.h"

using namespace divoomdev::common;
using namespace divoomdev::ics;
using namespace ics_test;
using namespace std::chrono;

namespace {

/// Fake calendar source: no real network, records how many times it is called.
class mock_source final : public i_calendar_source {
 public:
  int fetch_count = 0;
  std::map<std::string, std::string> contents;

  std::string fetch(const std::string& url) override {
    ++fetch_count;
    auto it = contents.find(url);
    return it == contents.end() ? std::string() : it->second;
  }
};

/// Builds an ICS document with a single UTC event starting at `start`.
std::string make_ics(const std::string& uid, const std::string& summary, time_point<system_clock> start) {
  auto end = start + std::chrono::hours(1);
  return "BEGIN:VCALENDAR\nBEGIN:VEVENT\nUID:" + uid + "\nSUMMARY:" + summary + "\nDTSTART:" + utc_dt(start) +
         "Z\nDTEND:" + utc_dt(end) + "Z\nSTATUS:CONFIRMED\nEND:VEVENT\nEND:VCALENDAR\n";
}

scheduled_event mk(const std::string& uid, const std::string& summary, time_point<system_clock> start) {
  scheduled_event ev;
  ev.uid = uid;
  ev.summary = summary;
  ev.start = start;
  ev.end = start + std::chrono::minutes(30);
  return ev;
}

}  // namespace

TEST(EventManager, LoadsFromMockSourceAndCaches) {
  auto source = std::make_shared<mock_source>();
  auto cache = std::make_shared<ics_cache>(std::chrono::minutes(5));
  auto now = system_clock::now();
  source->contents["http://a/cal.ics"] = make_ics("e1", "Cached Event", now + std::chrono::hours(1));

  event_manager mgr(source, cache);
  mgr.add_calendar_url("http://a/cal.ics");

  auto first = mgr.get_next_events();
  ASSERT_EQ(first.size(), 1u);
  EXPECT_EQ(first[0].summary, "Cached Event");
  EXPECT_EQ(source->fetch_count, 1);

  auto second = mgr.get_next_events();
  ASSERT_EQ(second.size(), 1u);
  EXPECT_EQ(source->fetch_count, 1);  // served from cache, no extra round-trip
}

TEST(EventManager, UsesManualEventsWithoutSourceFetch) {
  auto source = std::make_shared<mock_source>();
  event_manager mgr(source, nullptr);
  auto now = system_clock::now();

  mgr.add_event(mk("past", "Earlier", now - std::chrono::hours(1)));
  mgr.add_event(mk("soon", "Soon", now + std::chrono::hours(1)));
  mgr.add_event(mk("far", "Far", now + std::chrono::hours(100)));

  auto out = mgr.get_next_events_scheduled();
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0].uid, "past");
  EXPECT_EQ(out[1].uid, "soon");
  EXPECT_EQ(source->fetch_count, 0);
}

TEST(EventManager, ConfigurableHorizonViaSettings) {
  auto source = std::make_shared<mock_source>();
  auto now = system_clock::now();
  source->contents["u1"] = make_ics("h1", "Nearby", now + std::chrono::hours(2));

  calendar_settings settings;
  settings.horizon = std::chrono::hours(1);
  event_manager mgr(source, nullptr, settings);
  mgr.add_calendar_url("u1");

  EXPECT_TRUE(mgr.get_next_events().empty());
}

TEST(EventManager, FailedSourceYieldsNoEvents) {
  auto source = std::make_shared<mock_source>();
  event_manager mgr(source);
  mgr.add_calendar_url("http://missing/cal.ics");
  EXPECT_TRUE(mgr.get_next_events().empty());
}

TEST(EventManager, DefaultConstructorUsesHttpSource) {
  event_manager mgr;
  auto now = system_clock::now();
  mgr.add_event(mk("manual", "Manual Event", now + std::chrono::hours(1)));
  auto out = mgr.get_next_events_scheduled();
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(out[0].uid, "manual");
}