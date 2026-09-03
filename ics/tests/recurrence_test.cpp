#include "gtest/gtest.h"

#include "datetime.h"
#include "recurrence.h"
#include "test_helpers.h"
#include "timezone.h"

using namespace divoomdev::ics;
using namespace divoomdev::ics::detail;
using tp = std::chrono::time_point<std::chrono::system_clock>;

namespace {

scheduled_event make_event(tp start, tp end) {
  scheduled_event ev;
  ev.start = start;
  ev.end = end;
  ev.uid = "test";
  return ev;
}

std::vector<tp> expand_all(const scheduled_event& base,
                           const rrule_data& rule,
                           tp ws,
                           tp dl,
                           const std::set<std::string>& exdates = {}) {
  std::vector<scheduled_event> out;
  recurrence_expander::for_freq(rule.freq).expand(base, rule, ws, dl, exdates, out);
  std::vector<tp> starts;
  for (auto& e: out)
    starts.push_back(e.start);
  return starts;
}

}  // namespace

TEST(ParseRrule, Basic) {
  auto r = parse_rrule_data("FREQ=WEEKLY;INTERVAL=2;BYDAY=MO,WE,FR");
  EXPECT_EQ(r.freq, rrule_data::freq_t::WEEKLY);
  EXPECT_EQ(r.interval, 2);
  EXPECT_EQ(r.byday, (std::set<int>{1, 3, 5}));
}

TEST(ParseRrule, OrderIndependentAndCaseInsensitive) {
  auto r = parse_rrule_data("BYDAY=SU;INTERVAL=1;FREQ=DAILY");
  EXPECT_EQ(r.freq, rrule_data::freq_t::DAILY);
  EXPECT_EQ(r.byday, (std::set<int>{7}));
}

TEST(ParseRrule, MonthlyAndYearly) {
  EXPECT_EQ(parse_rrule_data("FREQ=MONTHLY").freq, rrule_data::freq_t::MONTHLY);
  EXPECT_EQ(parse_rrule_data("FREQ=YEARLY").freq, rrule_data::freq_t::YEARLY);
  EXPECT_EQ(parse_rrule_data("").freq, rrule_data::freq_t::DAILY);  // по умолчанию
}

TEST(ParseRrule, UntilUtc) {
  auto r = parse_rrule_data("FREQ=DAILY;UNTIL=20260905T120000Z");
  EXPECT_NE(r.until.time_since_epoch().count(), 0);
  EXPECT_EQ(ics_test::utc_dt(r.until), "20260905T120000");
}

TEST(DailyExpander, SimpleDaily) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 1, 10, 0, 0), ics_test::wall_clock(2026, 9, 1, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::DAILY;
  r.interval = 1;

  auto ws = ics_test::wall_clock(2026, 9, 2, 0, 0, 0);
  auto dl = ics_test::wall_clock(2026, 9, 5, 0, 0, 0);
  auto starts = expand_all(base, r, ws, dl);

  ASSERT_EQ(starts.size(), 3u);
  EXPECT_EQ(starts[0], base.start + std::chrono::hours(24));
  EXPECT_EQ(starts[1], base.start + std::chrono::hours(48));
  EXPECT_EQ(starts[2], base.start + std::chrono::hours(72));
}

TEST(DailyExpander, IntervalSkipsDays) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 1, 10, 0, 0), ics_test::wall_clock(2026, 9, 1, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::DAILY;
  r.interval = 2;
  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 6));
  ASSERT_EQ(starts.size(), 4u);  // +0, +2, +4, +6 дней (дедлайн включён)
  EXPECT_EQ(starts[0], base.start);
  EXPECT_EQ(starts[1], base.start + std::chrono::hours(48));
  EXPECT_EQ(starts[2], base.start + std::chrono::hours(96));
  EXPECT_EQ(starts[3], base.start + std::chrono::hours(144));
}

TEST(WeeklyExpander, BiweeklyAnchoredToDtstartNoOffWeek) {
  // Двухнедельная серия по пятницам должна давать только «правильные» недели.
  auto base = make_event(ics_test::wall_clock(2026, 9, 4, 10, 0, 0),  // пятница
                         ics_test::wall_clock(2026, 9, 4, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::WEEKLY;
  r.interval = 2;
  r.byday.insert(5);                                               // FR

  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 42));
  ASSERT_EQ(starts.size(), 4u);                                    // недели 0, 2, 4, 6 (дедлайн включён)
  EXPECT_EQ(starts[0], base.start);                                // неделя 0
  EXPECT_EQ(starts[1], base.start + std::chrono::hours(24 * 14));  // неделя 2
  EXPECT_EQ(starts[2], base.start + std::chrono::hours(24 * 28));  // неделя 4
  EXPECT_EQ(starts[3], base.start + std::chrono::hours(24 * 42));  // неделя 6

  // «Не та» пятница (неделя 1, +7 дней) не должна генерироваться.
  for (const auto& s: starts)
    EXPECT_NE(s, base.start + std::chrono::hours(24 * 7));
}

TEST(WeeklyExpander, MultipleBydayDays) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 4, 10, 0, 0), ics_test::wall_clock(2026, 9, 4, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::WEEKLY;
  r.interval = 1;
  r.byday.insert(5);  // FR
  r.byday.insert(6);  // SA

  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 9));
  ASSERT_EQ(starts.size(), 4u);
  EXPECT_EQ(starts[0], base.start);                               // FR неделя 0
  EXPECT_EQ(starts[1], base.start + std::chrono::hours(24));      // SA неделя 0
  EXPECT_EQ(starts[2], base.start + std::chrono::hours(24 * 7));  // FR неделя 1
  EXPECT_EQ(starts[3], base.start + std::chrono::hours(24 * 8));  // SA неделя 1
}

TEST(WeeklyExpander, DefaultDayOfDtstartWhenNoByday) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 4, 10, 0, 0), ics_test::wall_clock(2026, 9, 4, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::WEEKLY;
  r.interval = 1;  // без BYDAY — используется день DTSTART
  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 20));
  ASSERT_EQ(starts.size(), 3u);
  EXPECT_EQ(starts[0], base.start);
  EXPECT_EQ(starts[1], base.start + std::chrono::hours(24 * 7));
  EXPECT_EQ(starts[2], base.start + std::chrono::hours(24 * 14));
}

TEST(MonthlyExpander, AddsMonths) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 10, 10, 0, 0), ics_test::wall_clock(2026, 9, 10, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::MONTHLY;
  r.interval = 1;
  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 92));
  // Месячная серия включает базовый месяц (время обнуляется до 00:00).
  ASSERT_EQ(starts.size(), 4u);
  EXPECT_EQ(ics_test::fmt_dt(starts[0]), "2026-09-10 00:00:00");
  EXPECT_EQ(ics_test::fmt_dt(starts[1]), "2026-10-10 00:00:00");
  EXPECT_EQ(ics_test::fmt_dt(starts[2]), "2026-11-10 00:00:00");
  EXPECT_EQ(ics_test::fmt_dt(starts[3]), "2026-12-10 00:00:00");
}

TEST(Until, StopsExpansionInPast) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 1, 10, 0, 0), ics_test::wall_clock(2026, 9, 1, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::DAILY;
  r.until = ics_test::wall_clock(2026, 1, 1, 0, 0, 0);
  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 10));
  EXPECT_TRUE(starts.empty());
}

TEST(Until, FutureUntilDoesNotFilter) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 1, 10, 0, 0), ics_test::wall_clock(2026, 9, 1, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::DAILY;
  r.until = ics_test::wall_clock(2030, 1, 1, 0, 0, 0);
  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 4));
  ASSERT_EQ(starts.size(), 5u);  // 01..05 сентября
}

TEST(Exdate, ExcludesMatchingOccurrence) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 1, 10, 0, 0), ics_test::wall_clock(2026, 9, 1, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::DAILY;

  std::set<std::string> exdates;
  exdates.insert(format_local_dt(base.start + std::chrono::hours(48)));  // 03.09

  // Окно 09-01..09-06 (6 дней) минус исключённый 03.09 => 5 вхождений.
  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 5), exdates);
  ASSERT_EQ(starts.size(), 5u);
  for (const auto& s: starts)
    EXPECT_NE(s, base.start + std::chrono::hours(48));
}

TEST(IsExcluded, EmptySetNeverExcludes) {
  auto base = ics_test::wall_clock(2026, 9, 1, 10, 0, 0);
  EXPECT_FALSE(is_excluded(base, {}));
}

TEST(ApplyRepeatMetadata, PopulatesFields) {
  rrule_data r;
  r.freq = rrule_data::freq_t::WEEKLY;
  r.interval = 2;
  r.byday.insert(1);  // MO
  r.byday.insert(5);  // FR

  scheduled_event ev;
  apply_repeat_metadata(ev, r);
  EXPECT_EQ(ev.repeat, scheduled_event::repeat_t::WEEKLY);
  EXPECT_EQ(ev.recurrence_interval, 2);
  // биты MO(1) и FR(5): (1<<0)|(1<<4) = 17
  EXPECT_EQ(static_cast<int>(ev.byday_mask), 17);
}

TEST(Yearly, NoopExpander) {
  auto base = make_event(ics_test::wall_clock(2026, 9, 1, 10, 0, 0), ics_test::wall_clock(2026, 9, 1, 11, 0, 0));
  rrule_data r;
  r.freq = rrule_data::freq_t::YEARLY;
  auto starts = expand_all(base, r, base.start, base.start + std::chrono::hours(24 * 400));
  EXPECT_TRUE(starts.empty());
}
