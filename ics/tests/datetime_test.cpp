#include "gtest/gtest.h"

#include <sstream>

#include "datetime.h"
#include "test_helpers.h"
#include "text_utils.h"
#include "timezone.h"

using namespace divoomdev::ics::detail;

TEST(ParseTzOffset, Formats) {
  EXPECT_EQ(parse_tzoffset("+0300"), 3);
  EXPECT_EQ(parse_tzoffset("-0700"), -7);
  EXPECT_EQ(parse_tzoffset("+0530"), 6);  // получасовые округляются вверх
  EXPECT_EQ(parse_tzoffset("+0000"), 0);
  EXPECT_EQ(parse_tzoffset("+05"), kUnknownOffset);
  EXPECT_EQ(parse_tzoffset("abc"), kUnknownOffset);
}

TEST(MakeDatetimeInfo, TzidParam) {
  auto info = make_datetime_info("TZID=Europe/Moscow", "20231202T120000");
  EXPECT_EQ(info.timezone, "Europe/Moscow");
  EXPECT_FALSE(info.is_utc);
  EXPECT_FALSE(info.is_date);
}

TEST(MakeDatetimeInfo, UtcByTrailingZ) {
  auto info = make_datetime_info("", "20231202T120000Z");
  EXPECT_TRUE(info.is_utc);
  EXPECT_FALSE(info.is_date);
}

TEST(MakeDatetimeInfo, DateOnly) {
  auto info = make_datetime_info("VALUE=DATE", "20231202");
  EXPECT_TRUE(info.is_date);
  EXPECT_FALSE(info.is_utc);
}

TEST(MakeDatetimeInfo, MultipleParams) {
  auto info = make_datetime_info("TZID=GMT Standard Time;X=1", "20231202T120000");
  EXPECT_EQ(info.timezone, "GMT Standard Time");
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
  // +3 => 09:00 UTC
  EXPECT_EQ(ics_test::utc_dt(tp), "20231202T090000");
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

TEST(ParseIcsDatetime, UnknownTzUsesSystemLocal) {
  timezone_resolver tz;
  auto info = make_datetime_info("TZID=Nowhere/Unknown", "20231202T120000");
  EXPECT_EQ(tz.offset_hours("Nowhere/Unknown"), kUnknownOffset);
  auto tp = parse_ics_datetime(info, tz);
  // Полагаемся на системный пояс; просто проверяем, что не упало и время не пустое.
  EXPECT_NE(tp.time_since_epoch().count(), 0);
}

TEST(ParseStamp, UtcStamp) {
  auto tp = parse_stamp("20231202T120000Z");
  EXPECT_EQ(ics_test::utc_dt(tp), "20231202T120000");
}

TEST(FormatLocalDt, RoundTripShape) {
  auto tp = ics_test::wall_clock(2026, 9, 5, 10, 30, 0);
  std::string s = format_local_dt(tp);
  EXPECT_EQ(s.substr(0, 8), "20260905");
  EXPECT_EQ(s[8], 'T');
  EXPECT_EQ(s.substr(9), "103000");
}

TEST(TimezoneResolver, VTimezoneIgnoresDaylight) {
  // STANDARD берёт TZOFFSETTO; DAYLIGHT не перекрывает его для фиксированных поясов.
  std::string cal = "BEGIN:VCALENDAR\n"
                    "BEGIN:VTIMEZONE\n"
                    "TZID:SE Asia Standard Time\n"
                    "BEGIN:STANDARD\n"
                    "DTSTART:16010101T000000\n"
                    "TZOFFSETFROM:+0700\n"
                    "TZOFFSETTO:+0700\n"
                    "END:STANDARD\n"
                    "BEGIN:DAYLIGHT\n"
                    "DTSTART:16010101T000000\n"
                    "TZOFFSETFROM:+0700\n"
                    "TZOFFSETTO:+0700\n"
                    "END:DAYLIGHT\n"
                    "END:VTIMEZONE\n"
                    "END:VCALENDAR\n";
  timezone_resolver tz;
  tz.collect_vtimezones(cal);
  EXPECT_EQ(tz.offset_hours("SE Asia Standard Time"), 7);
}