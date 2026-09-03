#include "gtest/gtest.h"

#include <string>
#include <vector>

#include "common/event.h"
#include "event_builder.h"
#include "raw_event.h"
#include "test_helpers.h"
#include "text_utils.h"
#include "timezone.h"

namespace common = divoomdev::common;
using namespace divoomdev::ics;
using namespace divoomdev::ics::detail;

namespace {
const timezone_resolver kTz;
}

TEST(EventBuilder, ParsesCoreFields) {
  std::vector<std::string> lines = {
      "SUMMARY:HEMA Длинный меч - теория",
      "UID:h739VrXjyandex.ru",
      "LOCATION:Школа фехтования",
      "STATUS:TENTATIVE",
      "DTSTART;TZID=Europe/Moscow:20260904T100000",
      "DTEND;TZID=Europe/Moscow:20260904T110000",
      "ORGANIZER;CN=John Doe:mailto:john@example.com",
  };
  event_builder builder(kTz);
  auto re = builder.build(lines, {});

  EXPECT_EQ(re.ev.summary, "HEMA Длинный меч - теория");
  EXPECT_EQ(re.ev.uid, "h739VrXjyandex.ru");
  EXPECT_EQ(re.ev.location, "Школа фехтования");
  EXPECT_EQ(re.ev.status, common::event_status::TENTATIVE);
  EXPECT_EQ(re.ev.organizer, "john@example.com");
  EXPECT_EQ(re.ev.organizer_name, "John Doe");
  EXPECT_EQ(ics_test::utc_dt(re.ev.start), "20260904T070000");  // +3 -> UTC
  EXPECT_EQ(ics_test::utc_dt(re.ev.end), "20260904T080000");
}

TEST(EventBuilder, ParsesStatusCanceled) {
  std::vector<std::string> lines = {"UID:x", "STATUS:CANCELED", "SUMMARY:Отменено"};
  event_builder builder(kTz);
  auto re = builder.build(lines, {});
  EXPECT_EQ(re.ev.status, common::event_status::CANCELED);
}

TEST(EventBuilder, ParsesRruleAndExdates) {
  std::vector<std::string> lines = {
      "UID:r",
      "RRULE:FREQ=WEEKLY;INTERVAL=1;BYDAY=MO",
      "EXDATE;TZID=Europe/Moscow:20260911T100000,20260918T100000",
  };
  event_builder builder(kTz);
  auto re = builder.build(lines, {});

  EXPECT_TRUE(re.has_rrule);
  EXPECT_EQ(re.rrule.freq, rrule_data::freq_t::WEEKLY);
  ASSERT_EQ(re.exdates.size(), 2u);
  EXPECT_NE(re.exdates.find("20260911T100000"), re.exdates.end());
  EXPECT_NE(re.exdates.find("20260918T100000"), re.exdates.end());
  ASSERT_EQ(re.ev.exdates.size(), 2u);
}

TEST(EventBuilder, ParsesRecurrenceId) {
  std::vector<std::string> lines = {
      "UID:r",
      "RECURRENCE-ID;TZID=Europe/Moscow:20260911T100000",
  };
  event_builder builder(kTz);
  auto re = builder.build(lines, {});
  EXPECT_TRUE(re.has_recurrence_id);
  EXPECT_EQ(re.recurrence_id, "20260911T100000");
}

TEST(EventBuilder, ParsesSequenceAndUrl) {
  std::vector<std::string> lines = {"UID:s", "SEQUENCE:5", "URL:https://example.com/e"};
  event_builder builder(kTz);
  auto re = builder.build(lines, {});
  EXPECT_EQ(re.ev.sequence, 5);
  EXPECT_EQ(re.ev.url, "https://example.com/e");
}

TEST(EventBuilder, ParsesAlarm) {
  std::vector<std::string> lines = {"UID:a", "SUMMARY:Событие"};
  std::vector<std::string> alarm_lines = {"ACTION:DISPLAY", "TRIGGER:-PT30M", "DESCRIPTION:Пора!"};
  event_builder builder(kTz);
  auto re = builder.build(lines, alarm_lines);

  ASSERT_EQ(re.ev.alarms.size(), 1u);
  EXPECT_EQ(re.ev.alarms[0].action, "DISPLAY");
  EXPECT_EQ(re.ev.alarms[0].description, "Пора!");
  EXPECT_EQ(re.ev.alarms[0].trigger_minutes, std::chrono::minutes(30));
}

TEST(EventBuilder, UnescapesSummary) {
  // \n -> перевод строки; кавычка не входит в набор экранируемых символов, поэтому
  // обратный слэш перед ней сохраняется (таково текущее поведение библиотеки).
  std::vector<std::string> lines = {"UID:u", "SUMMARY:Ревью \\\"RAID1\\\" и \\nперерыв"};
  event_builder builder(kTz);
  auto re = builder.build(lines, {});
  EXPECT_EQ(re.ev.summary, "Ревью \\\"RAID1\\\" и \nперерыв");
}

TEST(EventBuilder, IgnoresUnknownKeys) {
  std::vector<std::string> lines = {"UID:u", "X-CUSTOM-PROP:whatever", "ATTENDEE:mailto:x@y.z"};
  event_builder builder(kTz);
  auto re = builder.build(lines, {});
  EXPECT_EQ(re.ev.uid, "u");
}