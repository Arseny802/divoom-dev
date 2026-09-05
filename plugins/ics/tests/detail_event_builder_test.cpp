#include "gtest/gtest.h"

#include <string>
#include <vector>

#include "common/event.h"
#include "detail/event_builder.h"
#include "detail/raw_event.h"
#include "detail/text_utils.h"
#include "detail/timezone.h"
#include "test_helpers.h"

using namespace divoomdev::common;
using namespace divoomdev::ics;
using namespace divoomdev::ics::detail;

namespace {
const timezone_resolver kTz;
}

TEST(EventBuilder, ParsesCoreFields) {
  std::vector<std::string> lines = {
      "SUMMARY:Team Sync",
      "UID:h739VrXj",
      "LOCATION:Room 4",
      "STATUS:TENTATIVE",
      "DTSTART;TZID=Europe/Moscow:20260904T100000",
      "DTEND;TZID=Europe/Moscow:20260904T110000",
      "ORGANIZER;CN=John Doe:mailto:john@example.com",
  };
  event_builder builder(kTz);
  auto re = builder.build(lines, {});

  EXPECT_EQ(re.ev.summary, "Team Sync");
  EXPECT_EQ(re.ev.uid, "h739VrXj");
  EXPECT_EQ(re.ev.location, "Room 4");
  EXPECT_EQ(re.ev.status, event_status::TENTATIVE);
  EXPECT_EQ(re.ev.organizer, "john@example.com");
  EXPECT_EQ(re.ev.organizer_name, "John Doe");
  EXPECT_EQ(ics_test::utc_dt(re.ev.start), "20260904T070000");  // +3 -> UTC
  EXPECT_EQ(ics_test::utc_dt(re.ev.end), "20260904T080000");
}

TEST(EventBuilder, ParsesStatusCanceled) {
  event_builder builder(kTz);
  auto re = builder.build({"UID:x", "STATUS:CANCELED", "SUMMARY:Canceled"}, {});
  EXPECT_EQ(re.ev.status, event_status::CANCELED);
}

TEST(EventBuilder, ParsesRruleAndMultiValueExdate) {
  event_builder builder(kTz);
  auto re = builder.build(
      {"UID:r", "RRULE:FREQ=WEEKLY;INTERVAL=1;BYDAY=MO", "EXDATE;TZID=Europe/Moscow:20260911T100000,20260918T100000"},
      {});
  EXPECT_TRUE(re.has_rrule);
  EXPECT_EQ(re.rrule.freq, rrule_data::freq_t::WEEKLY);
  ASSERT_EQ(re.exdates.size(), 2u);
  EXPECT_NE(re.exdates.find("20260911T100000"), re.exdates.end());
  EXPECT_NE(re.exdates.find("20260918T100000"), re.exdates.end());
}

TEST(EventBuilder, ParsesRecurrenceId) {
  event_builder builder(kTz);
  auto re = builder.build({"UID:r", "RECURRENCE-ID;TZID=Europe/Moscow:20260911T100000"}, {});
  EXPECT_TRUE(re.has_recurrence_id);
  EXPECT_EQ(re.recurrence_id, "20260911T100000");
}

TEST(EventBuilder, ParsesSequenceAndUrl) {
  event_builder builder(kTz);
  auto re = builder.build({"UID:s", "SEQUENCE:5", "URL:https://example.com/e"}, {});
  EXPECT_EQ(re.ev.sequence, 5);
  EXPECT_EQ(re.ev.url, "https://example.com/e");
}

TEST(EventBuilder, ParsesAlarm) {
  event_builder builder(kTz);
  auto re =
      builder.build({"UID:a", "SUMMARY:Reminder"}, {"ACTION:DISPLAY", "TRIGGER:-PT30M", "DESCRIPTION:Prepare slides"});
  ASSERT_EQ(re.ev.alarms.size(), 1u);
  EXPECT_EQ(re.ev.alarms[0].action, "DISPLAY");
  EXPECT_EQ(re.ev.alarms[0].description, "Prepare slides");
  EXPECT_EQ(re.ev.alarms[0].trigger_minutes, std::chrono::minutes(30));
}

TEST(EventBuilder, UnescapesSupportedSequences) {
  // \n becomes a newline; a backslash before an unsupported char is preserved.
  event_builder builder(kTz);
  auto re = builder.build({"UID:u", "SUMMARY:line1\\nline2"}, {});
  EXPECT_EQ(re.ev.summary, "line1\nline2");
}

TEST(EventBuilder, IgnoresUnknownKeys) {
  event_builder builder(kTz);
  auto re = builder.build({"UID:u", "X-CUSTOM-PROP:whatever", "ATTENDEE:mailto:x@y.z"}, {});
  EXPECT_EQ(re.ev.uid, "u");
}