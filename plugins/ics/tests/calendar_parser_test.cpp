#include "gtest/gtest.h"

#include <algorithm>
#include <string>

#include "detail/text_utils.h"
#include "ics/parser.h"
#include "ics/settings.h"
#include "test_helpers.h"

using namespace divoomdev::ics;
using namespace ics_test;

namespace {
// Common "now" anchor used by most fixtures.
const auto kNow = ics_test::wall_clock(2026, 9, 4, 8, 0, 0);

scheduled_event_list parse_fixture(const std::string& name,
                                   std::chrono::time_point<std::chrono::system_clock> now = kNow,
                                   calendar_settings settings = {}) {
  calendar_parser parser(settings);
  return parser.parse_at(fixture(name), now);
}
}  // namespace

TEST(CalendarParser, SingleEventParsed) {
  auto events = parse_fixture("single_event.ics");
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].summary, "Morning Standup");
  EXPECT_EQ(ics_test::utc_dt(events[0].start), "20260904T070000");
}

TEST(CalendarParser, CanceledEventExcluded) {
  EXPECT_TRUE(parse_fixture("canceled_event.ics").empty());
}

TEST(CalendarParser, FoldedSummaryAndRruleMerged) {
  auto events = parse_fixture("folded_summary.ics");
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].summary, "Team Sync - very long summary that gets folded across multiple lines without loss");
  EXPECT_EQ(ics_test::utc_dt(events[0].start), "20260904T083000");
}

TEST(CalendarParser, ExdateSkipsOccurrence) {
  auto now = ics_test::wall_clock(2026, 9, 1, 12, 0, 0);
  auto events = parse_fixture("daily_exdate.ics", now);
  ASSERT_EQ(events.size(), 2u);
  for (const auto& e: events)
    EXPECT_NE(ics_test::utc_dt(e.start).substr(0, 8), "20260902");
}

TEST(CalendarParser, BiweeklyFridaySkipsOffWeek) {
  calendar_settings settings;
  settings.horizon = std::chrono::hours(24 * 20);

  auto events = parse_fixture("biweekly_friday.ics", kNow, settings);
  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(ics_test::fmt_dt(events[0].start), "2026-09-04 10:00:00");
  EXPECT_EQ(ics_test::fmt_dt(events[1].start), "2026-09-18 10:00:00");
}

TEST(CalendarParser, UntilStopsSeries) {
  auto events = parse_fixture("until.ics");
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(ics_test::fmt_dt(events[0].start), "2026-09-04 10:00:00");
}

TEST(CalendarParser, RecurrenceIdReplacesInstanceWithoutDuplicate) {
  auto now = ics_test::wall_clock(2026, 9, 2, 12, 0, 0);
  auto events = parse_fixture("override_move.ics", now);
  ASSERT_EQ(events.size(), 3u);
  EXPECT_EQ(ics_test::fmt_dt(events[1].start), "2026-09-03 14:00:00");
  for (const auto& e: events)
    EXPECT_NE(ics_test::fmt_dt(e.start), "2026-09-03 10:00:00");
}

TEST(CalendarParser, CanceledOverrideDropsInstance) {
  auto now = ics_test::wall_clock(2026, 9, 2, 12, 0, 0);
  auto events = parse_fixture("override_cancel.ics", now);
  ASSERT_EQ(events.size(), 2u);
  for (const auto& e: events)
    EXPECT_NE(ics_test::fmt_dt(e.start), "2026-09-03 10:00:00");
}

TEST(CalendarParser, OutlookWindowsTzidFourEvents) {
  auto events = parse_fixture("outlook_windows.ics");
  ASSERT_EQ(events.size(), 4u);
  EXPECT_EQ(events[0].summary, "Interview Alex Carter");
  EXPECT_EQ(ics_test::utc_dt(events[0].start), "20260904T070000");
  EXPECT_EQ(events[1].summary, "Daily Sync Alpha");
  EXPECT_EQ(ics_test::utc_dt(events[1].start), "20260904T083000");
  EXPECT_EQ(events[2].summary, "Daily Lab Cpp");
  EXPECT_EQ(events[3].summary, "Architecture Review");
  EXPECT_EQ(ics_test::utc_dt(events[3].start), "20260904T130000");
}

TEST(CalendarParser, NonAsciiSummaryPreserved) {
  auto expected = detail::trim(ics_test::fixture("unicode_summary_expected.txt"));
  auto now = ics_test::wall_clock(2026, 9, 5, 8, 0, 0);

  auto events = parse_fixture("unicode_summary.ics", now);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].summary, expected);
}

TEST(CalendarParser, AlarmAttached) {
  auto events = parse_fixture("alarm_event.ics");
  ASSERT_EQ(events.size(), 1u);
  ASSERT_EQ(events[0].alarms.size(), 1u);
  EXPECT_EQ(events[0].alarms[0].action, "DISPLAY");
  EXPECT_EQ(events[0].alarms[0].trigger_minutes, std::chrono::minutes(30));
}

TEST(CalendarParser, ResultsSortedByStart) {
  auto events = parse_fixture("sort_events.ics");
  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(events[0].uid, "sort-early");
  EXPECT_EQ(events[1].uid, "sort-late");
  EXPECT_TRUE(std::is_sorted(events.begin(), events.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  }));
}

TEST(CalendarParser, DateOnlyEventAtLocalMidnight) {
  auto events = parse_fixture("date_only.ics");
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(ics_test::fmt_dt(events[0].start), "2026-09-04 00:00:00");
}

TEST(CalendarParser, EmptyContentYieldsNoEvents) {
  EXPECT_TRUE(parse_fixture("__nonexistent__.ics").empty());
}

TEST(CalendarParser, ConfigurableSettingsAffectWindow) {
  calendar_settings narrow;
  narrow.horizon = std::chrono::hours(1);
  auto now = ics_test::wall_clock(2026, 9, 4, 8, 0, 0);
  auto events = parse_fixture("single_event.ics", now, narrow);
  EXPECT_TRUE(events.empty());
}