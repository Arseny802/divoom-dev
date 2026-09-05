#include "gtest/gtest.h"

#include "detail/datetime.h"
#include "detail/text_utils.h"
#include "detail/timezone.h"
#include "test_helpers.h"

using namespace divoomdev::ics::detail;

TEST(ParseTzOffset, HandlesFormats) {
  EXPECT_EQ(parse_tzoffset("+0300"), 3);
  EXPECT_EQ(parse_tzoffset("-0700"), -7);
  EXPECT_EQ(parse_tzoffset("+0530"), 6);  // half-hour offsets round up
  EXPECT_EQ(parse_tzoffset("+0000"), 0);
  EXPECT_EQ(parse_tzoffset("+05"), kUnknownOffset);
  EXPECT_EQ(parse_tzoffset("abc"), kUnknownOffset);
}

TEST(MakeDatetimeInfo, TzidAndUtcAndDateFlags) {
  auto with_tz = make_datetime_info("TZID=Europe/Moscow", "20231202T120000");
  EXPECT_EQ(with_tz.timezone, "Europe/Moscow");
  EXPECT_FALSE(with_tz.is_utc);

  auto utc = make_datetime_info("", "20231202T120000Z");
  EXPECT_TRUE(utc.is_utc);

  auto date = make_datetime_info("VALUE=DATE", "20231202");
  EXPECT_TRUE(date.is_date);
}

TEST(ParseDatetimeField, FullLine) {
  auto info = parse_datetime_field("DTSTART;TZID=Russian Standard Time:20231202T120000");
  EXPECT_EQ(info.timezone, "Russian Standard Time");
  EXPECT_EQ(info.value, "20231202T120000");
}

TEST(ParseIcsDatetime, UtcIsAbsolute) {
  timezone_resolver tz;
  auto tp = parse_ics_datetime(make_datetime_info("", "20231202T120000Z"), tz);
  EXPECT_EQ(ics_test::utc_dt(tp), "20231202T120000");
}

TEST(ParseIcsDatetime, IanaStaticTimezone) {
  timezone_resolver tz;
  auto tp = parse_ics_datetime(make_datetime_info("TZID=Europe/Moscow", "20231202T120000"), tz);
  EXPECT_EQ(ics_test::utc_dt(tp), "20231202T090000");  // +3 -> UTC
}

TEST(ParseIcsDatetime, WindowsTzidFromVTimezone) {
  std::string cal = "BEGIN:VCALENDAR\n"
                    "BEGIN:VTIMEZONE\n"
                    "TZID:Russian Standard Time\n"
                    "BEGIN:STANDARD\n"
                    "TZOFFSETFROM:+0300\n"
                    "TZOFFSETTO:+0300\n"
                    "END:STANDARD\n"
                    "END:VTIMEZONE\n"
                    "END:VCALENDAR\n";
  timezone_resolver tz;
  tz.collect_vtimezones(cal);
  EXPECT_EQ(tz.offset_hours("Russian Standard Time"), 3);

  auto tp = parse_ics_datetime(make_datetime_info("TZID=Russian Standard Time", "20231202T120000"), tz);
  EXPECT_EQ(ics_test::utc_dt(tp), "20231202T090000");
}

TEST(ParseIcsDatetime, UnknownTzFallsBackToSystemLocal) {
  timezone_resolver tz;
  auto tp = parse_ics_datetime(make_datetime_info("TZID=Nowhere/Unknown", "20231202T120000"), tz);
  EXPECT_NE(tp.time_since_epoch().count(), 0);
}

TEST(ParseStamp, UtcStamp) {
  auto tp = parse_stamp("20231202T120000Z");
  EXPECT_EQ(ics_test::utc_dt(tp), "20231202T120000");
}

TEST(FormatLocalDt, ShapeRoundTrip) {
  auto tp = ics_test::wall_clock(2026, 9, 5, 10, 30, 0);
  std::string s = format_local_dt(tp);
  EXPECT_EQ(s.substr(0, 8), "20260905");
  EXPECT_EQ(s[8], 'T');
  EXPECT_EQ(s.substr(9), "103000");
}

TEST(TimezoneResolver, IgnoresDaylightForFixedZones) {
  std::string cal = "BEGIN:VCALENDAR\n"
                    "BEGIN:VTIMEZONE\n"
                    "TZID:SE Asia Standard Time\n"
                    "BEGIN:STANDARD\n"
                    "TZOFFSETFROM:+0700\n"
                    "TZOFFSETTO:+0700\n"
                    "END:STANDARD\n"
                    "BEGIN:DAYLIGHT\n"
                    "TZOFFSETFROM:+0700\n"
                    "TZOFFSETTO:+0700\n"
                    "END:DAYLIGHT\n"
                    "END:VTIMEZONE\n"
                    "END:VCALENDAR\n";
  timezone_resolver tz;
  tz.collect_vtimezones(cal);
  EXPECT_EQ(tz.offset_hours("SE Asia Standard Time"), 7);
}