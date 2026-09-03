#include "gtest/gtest.h"

#include <algorithm>
#include <string>
#include <vector>

#include "calendar_parser.h"
#include "test_helpers.h"
#include "window.h"

using namespace divoomdev::ics;
using namespace divoomdev::ics::detail;

namespace {

// Outlook-формат (Windows TZID + folded RRULE/SUMMARY): ровно 4 события 2026-09-04.
const std::string kOutlookCalendar = "BEGIN:VCALENDAR\r\n"
                                     "PRODID:Microsoft Exchange Server 2010\r\n"
                                     "VERSION:2.0\r\n"
                                     "BEGIN:VTIMEZONE\r\n"
                                     "TZID:Russian Standard Time\r\n"
                                     "BEGIN:STANDARD\r\n"
                                     "DTSTART:16010101T000000\r\n"
                                     "TZOFFSETFROM:+0300\r\n"
                                     "TZOFFSETTO:+0300\r\n"
                                     "END:STANDARD\r\n"
                                     "END:VTIMEZONE\r\n"
                                     "BEGIN:VEVENT\r\n"
                                     "UID:o1\r\n"
                                     "SUMMARY:Собеседование Киричёк Олег (команда)\r\n"
                                     "DTSTART;TZID=Russian Standard Time:20260904T100000\r\n"
                                     "DTEND;TZID=Russian Standard Time:20260904T110000\r\n"
                                     "STATUS:CONFIRMED\r\n"
                                     "END:VEVENT\r\n"
                                     "BEGIN:VEVENT\r\n"
                                     "RRULE:FREQ=WEEKLY;INTERVAL=1;BYDAY=MO,TU,WE,TH,FR;WKST=\r\n"
                                     " MO\r\n"
                                     "UID:o2\r\n"
                                     "SUMMARY:Daily M6 (команда) \r\n"
                                     "DTSTART;TZID=Russian Standard Time:20260904T113000\r\n"
                                     "DTEND;TZID=Russian Standard Time:20260904T120000\r\n"
                                     "STATUS:CONFIRMED\r\n"
                                     "END:VEVENT\r\n"
                                     "BEGIN:VEVENT\r\n"
                                     "UID:o3\r\n"
                                     "SUMMARY:Daily Lab C++\r\n"
                                     "DTSTART;TZID=Russian Standard Time:20260904T123000\r\n"
                                     "DTEND;TZID=Russian Standard Time:20260904T133000\r\n"
                                     "END:VEVENT\r\n"
                                     "BEGIN:VEVENT\r\n"
                                     "UID:o4\r\n"
                                     "SUMMARY:Арх. ревью «RAID1»\r\n"
                                     "DTSTART;TZID=Russian Standard Time:20260904T160000\r\n"
                                     "DTEND;TZID=Russian Standard Time:20260904T170000\r\n"
                                     "END:VEVENT\r\n"
                                     "END:VCALENDAR\r\n";

}  // namespace

TEST(ParseCalendar, SingleOneOffEvent) {
  const std::string cal = "BEGIN:VCALENDAR\n"
                          "BEGIN:VEVENT\n"
                          "UID:one\n"
                          "SUMMARY:Единственное\n"
                          "DTSTART;TZID=Europe/Moscow:20260904T100000\n"
                          "DTEND;TZID=Europe/Moscow:20260904T110000\n"
                          "END:VEVENT\n"
                          "END:VCALENDAR\n";

  auto now = ics_test::wall_clock(2026, 9, 4, 9, 0, 0);
  auto events = parse_calendar_at(cal, now);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].summary, "Единственное");
  EXPECT_EQ(ics_test::utc_dt(events[0].start), "20260904T070000");
}

TEST(ParseCalendar, ExcludesCanceledEvents) {
  const std::string cal = "BEGIN:VCALENDAR\n"
                          "BEGIN:VEVENT\n"
                          "UID:c\n"
                          "SUMMARY:Отменено\n"
                          "STATUS:CANCELED\n"
                          "DTSTART;TZID=Europe/Moscow:20260904T100000\n"
                          "DTEND;TZID=Europe/Moscow:20260904T110000\n"
                          "END:VEVENT\n"
                          "END:VCALENDAR\n";
  auto now = ics_test::wall_clock(2026, 9, 4, 9, 0, 0);
  auto events = parse_calendar_at(cal, now);
  EXPECT_TRUE(events.empty());
}

TEST(ParseCalendar, MergesFoldedSummary) {
  const std::string cal = "BEGIN:VCALENDAR\n"
                          "BEGIN:VEVENT\n"
                          "UID:f\n"
                          "SUMMARY:Собрание команды\n"
                          " :еженедельно в 11:30 без потери текста\n"
                          "DTSTART;TZID=Europe/Moscow:20260904T113000\n"
                          "DTEND;TZID=Europe/Moscow:20260904T120000\n"
                          "END:VEVENT\n"
                          "END:VCALENDAR\n";
  auto now = ics_test::wall_clock(2026, 9, 4, 9, 0, 0);
  auto events = parse_calendar_at(cal, now);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].summary, "Собрание команды:еженедельно в 11:30 без потери текста");
}

TEST(ParseCalendar, ExdateSkipsRecurringInstance) {
  const std::string cal = "BEGIN:VCALENDAR\n"
                          "BEGIN:VEVENT\n"
                          "UID:daily\n"
                          "SUMMARY:Ежедневная\n"
                          "RRULE:FREQ=DAILY\n"
                          "DTSTART;TZID=Europe/Moscow:20260901T090000\n"
                          "DTEND;TZID=Europe/Moscow:20260901T093000\n"
                          "EXDATE;TZID=Europe/Moscow:20260902T090000\n"
                          "END:VEVENT\n"
                          "END:VCALENDAR\n";
  auto now = ics_test::wall_clock(2026, 9, 1, 0, 30, 0);
  auto events = parse_calendar_at(cal, now);
  ASSERT_EQ(events.size(), 1u);  // 02.09 исключён EXDATE
  EXPECT_EQ(ics_test::utc_dt(events[0].start), "20260901T060000");
}

TEST(ParseCalendar, RecurrenceIdReplacesInstanceWithoutDuplicates) {
  const std::string cal = "BEGIN:VCALENDAR\n"
                          "BEGIN:VEVENT\n"
                          "UID:series\n"
                          "SUMMARY:Сериал\n"
                          "RRULE:FREQ=DAILY\n"
                          "DTSTART;TZID=Europe/Moscow:20260901T100000\n"
                          "DTEND;TZID=Europe/Moscow:20260901T110000\n"
                          "END:VEVENT\n"
                          "BEGIN:VEVENT\n"
                          "UID:series\n"
                          "SUMMARY:Сериал (перенос)\n"
                          "RECURRENCE-ID;TZID=Europe/Moscow:20260903T100000\n"
                          "DTSTART;TZID=Europe/Moscow:20260903T140000\n"
                          "DTEND;TZID=Europe/Moscow:20260903T150000\n"
                          "END:VEVENT\n"
                          "END:VCALENDAR\n";
  auto now = ics_test::wall_clock(2026, 9, 2, 12, 0, 0);
  auto events = parse_calendar_at(cal, now);

  ASSERT_EQ(events.size(), 3u);  // 02, 03(перенесённое), 04
  EXPECT_EQ(ics_test::fmt_dt(events[0].start), "2026-09-02 10:00:00");
  EXPECT_EQ(events[0].summary, "Сериал");
  EXPECT_EQ(ics_test::fmt_dt(events[1].start), "2026-09-03 14:00:00");
  EXPECT_EQ(events[1].summary, "Сериал (перенос)");
  EXPECT_EQ(ics_test::fmt_dt(events[2].start), "2026-09-04 10:00:00");

  // Нет дубликата оригинального вхождения 03.09 в 10:00.
  for (const auto& e: events)
    EXPECT_NE(ics_test::fmt_dt(e.start), "2026-09-03 10:00:00");
}

TEST(ParseCalendar, CanceledOverrideDropsInstance) {
  const std::string cal = "BEGIN:VCALENDAR\n"
                          "BEGIN:VEVENT\n"
                          "UID:series\n"
                          "SUMMARY:Сериал\n"
                          "RRULE:FREQ=DAILY\n"
                          "DTSTART;TZID=Europe/Moscow:20260901T100000\n"
                          "DTEND;TZID=Europe/Moscow:20260901T110000\n"
                          "END:VEVENT\n"
                          "BEGIN:VEVENT\n"
                          "UID:series\n"
                          "SUMMARY:Сериал\n"
                          "RECURRENCE-ID;TZID=Europe/Moscow:20260903T100000\n"
                          "DTSTART;TZID=Europe/Moscow:20260903T100000\n"
                          "DTEND;TZID=Europe/Moscow:20260903T110000\n"
                          "STATUS:CANCELED\n"
                          "END:VEVENT\n"
                          "END:VCALENDAR\n";
  auto now = ics_test::wall_clock(2026, 9, 2, 12, 0, 0);
  auto events = parse_calendar_at(cal, now);
  ASSERT_EQ(events.size(), 2u);  // 02 и 04; 03 отменён
  for (const auto& e: events)
    EXPECT_NE(ics_test::fmt_dt(e.start), "2026-09-03 10:00:00");
}

TEST(ParseCalendar, SortsByStart) {
  const std::string cal = "BEGIN:VCALENDAR\n"
                          "BEGIN:VEVENT\n"
                          "UID:e2\n"
                          "SUMMARY:Позже\n"
                          "DTSTART;TZID=Europe/Moscow:20260904T140000\n"
                          "DTEND;TZID=Europe/Moscow:20260904T150000\n"
                          "END:VEVENT\n"
                          "BEGIN:VEVENT\n"
                          "UID:e1\n"
                          "SUMMARY:Раньше\n"
                          "DTSTART;TZID=Europe/Moscow:20260904T090000\n"
                          "DTEND;TZID=Europe/Moscow:20260904T100000\n"
                          "END:VEVENT\n"
                          "END:VCALENDAR\n";
  auto now = ics_test::wall_clock(2026, 9, 4, 8, 0, 0);
  auto events = parse_calendar_at(cal, now);
  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(events[0].summary, "Раньше");
  EXPECT_EQ(events[1].summary, "Позже");
  EXPECT_TRUE(std::is_sorted(events.begin(), events.end(), [](const scheduled_event& a, const scheduled_event& b) {
    return a.start < b.start;
  }));
}

TEST(ParseCalendar, OutlookFormatReturnsExactlyFourEvents) {
  auto now = ics_test::wall_clock(2026, 9, 4, 8, 0, 0);
  auto events = parse_calendar_at(kOutlookCalendar, now);

  ASSERT_EQ(events.size(), 4u);
  EXPECT_EQ(events[0].summary, "Собеседование Киричёк Олег (команда)");
  EXPECT_EQ(ics_test::utc_dt(events[0].start), "20260904T070000");  // 10:00 MSK
  EXPECT_EQ(events[1].summary, "Daily M6 (команда)");
  EXPECT_EQ(ics_test::utc_dt(events[1].start), "20260904T083000");  // 11:30 MSK
  EXPECT_EQ(events[2].summary, "Daily Lab C++");
  EXPECT_EQ(ics_test::utc_dt(events[2].start), "20260904T093000");  // 12:30 MSK
  EXPECT_EQ(events[3].summary, "Арх. ревью «RAID1»");
  EXPECT_EQ(ics_test::utc_dt(events[3].start), "20260904T130000");  // 16:00 MSK
}

TEST(ParseCalendar, RealCalendarIcsProducesWeekendEvents) {
  std::string content = ics_test::read_file("../calendar.ics");
  ASSERT_FALSE(content.empty());

  auto now = ics_test::wall_clock(2026, 9, 5, 8, 0, 0);  // суббота
  auto events = parse_calendar_at(content, now);

  std::set<std::string> summaries;
  for (const auto& e: events)
    summaries.insert(e.summary);

  EXPECT_GE(events.size(), 3u);
  EXPECT_NE(summaries.find("Стрельба из лука"), summaries.end());
  EXPECT_NE(summaries.find("HEMA Свободные бои"), summaries.end());
  EXPECT_NE(summaries.find("HEMA Длинный меч - теория"), summaries.end());
}

TEST(ParseCalendar, RealOutlookIcsParsesFoldedLinesAndTzid) {
  std::string content = ics_test::read_file("../outlook3.ics");
  ASSERT_FALSE(content.empty());

  // Период активности "Daily M6" (DTSTART 2025-09-04, серия до 2026-02-06).
  auto now = ics_test::wall_clock(2025, 9, 4, 8, 0, 0);
  auto events = parse_calendar_at(content, now);

  ASSERT_FALSE(events.empty());
  // Все заголовки не пустые — folding не теряет SUMMARY.
  for (const auto& e: events)
    EXPECT_FALSE(e.summary.empty());

  bool found_m6 = false;
  for (const auto& e: events)
    if (e.summary.find("Daily M6 (команда)") != std::string::npos)
      found_m6 = true;
  EXPECT_TRUE(found_m6);
}