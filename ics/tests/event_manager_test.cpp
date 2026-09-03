#include "gtest/gtest.h"

#include <chrono>

#include "common/event.h"
#include "ics/event_manager.h"
#include "test_helpers.h"

namespace common = divoomdev::common;
using namespace divoomdev::ics;
using namespace std::chrono;

namespace {
scheduled_event mk(const std::string& uid, const std::string& summary, time_point<system_clock> start) {
  scheduled_event ev;
  ev.uid = uid;
  ev.summary = summary;
  ev.start = start;
  ev.end = start + std::chrono::minutes(30);
  return ev;
}
}  // namespace

TEST(EventManager, AddEventAndGetNext48hScheduledFiltersByDeadline) {
  event_manager mgr;
  auto now = system_clock::now();

  mgr.add_event(mk("e1", "прошлое", now - hours(1)));
  mgr.add_event(mk("e2", "скоро", now + hours(1)));
  mgr.add_event(mk("e3", "далеко", now + hours(100)));

  auto out = mgr.get_next_48h_scheduled();
  // get_next_48h_scheduled фильтрует только по верхней границе (deadline).
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0].uid, "e1");
  EXPECT_EQ(out[1].uid, "e2");
  EXPECT_TRUE(std::is_sorted(
      out.begin(), out.end(), [](const scheduled_event& a, const scheduled_event& b) { return a.start < b.start; }));
}

TEST(EventManager, GetNext48hEventsAppliesLowerBoundStartOfDay) {
  event_manager mgr;
  auto now = system_clock::now();

  mgr.add_event(mk("past", "давно начатое", now - hours(50)));  // раньше начала сегодняшнего дня
  mgr.add_event(mk("inside", "внутри окна", now + hours(1)));
  mgr.add_event(mk("far", "за пределами", now + hours(100)));   // за 48ч

  common::event_list out = mgr.get_next_48h_events();
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(out[0].uid, "inside");
}

TEST(EventManager, AddCalendarUrlThenAddEventNoFetchNeeded) {
  event_manager mgr;
  auto now = system_clock::now();
  mgr.add_event(mk("a", "A", now + minutes(5)));
  mgr.add_event(mk("b", "B", now + minutes(3)));
  // Без добавленных URL fetch не выполняется, события берутся из кэша.
  auto out = mgr.get_next_48h_scheduled();
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0].uid, "b");
  EXPECT_EQ(out[1].uid, "a");
}