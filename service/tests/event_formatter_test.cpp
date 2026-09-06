#include "gtest/gtest.h"

#include <chrono>
#include <ctime>
#include <string>
#include <vector>

#include "common/event.h"
#include "core/event_formatter.h"
#include "ics/scheduled_event.h"

using namespace divoomdev::service::core;
using namespace divoomdev::common;
using namespace divoomdev::ics;
using namespace std::chrono;

namespace {

static std::string to_local_str(time_point<system_clock> tp, const char* fmt) {
  auto t = system_clock::to_time_t(tp);
  struct tm tm;
  localtime_s(&tm, &t);
  char buf[64];
  strftime(buf, sizeof(buf), fmt, &tm);
  return std::string(buf);
}

static event make_event(const std::string& summary,
                        int start_h,
                        int start_m,
                        int end_h,
                        int end_m,
                        const std::string& desc = "",
                        const std::string& loc = "") {
  event ev;
  ev.summary = summary;
  ev.description = desc;
  ev.location = loc;

  auto now = system_clock::now();
  auto t = system_clock::to_time_t(now);
  struct tm tm;
  localtime_s(&tm, &t);
  tm.tm_hour = start_h;
  tm.tm_min = start_m;
  tm.tm_sec = 0;
  ev.start = system_clock::from_time_t(mktime(&tm));

  tm.tm_hour = end_h;
  tm.tm_min = end_m;
  tm.tm_sec = 0;
  ev.end = system_clock::from_time_t(mktime(&tm));

  return ev;
}

}  // namespace

// ============================================================================
// format_local_time
// ============================================================================
TEST(EventFormatter, FormatLocalTimeProducesExpectedFormat) {
  auto now = system_clock::now();
  auto time_str = format_local_time(now, "%H:%M");
  auto date_str = format_local_time(now, "%Y-%m-%d");

  // Time should be HH:MM (5 chars)
  EXPECT_EQ(time_str.size(), 5u);
  // Date should be YYYY-MM-DD (10 chars)
  EXPECT_EQ(date_str.size(), 10u);
}

TEST(EventFormatter, FormatLocalTimeWithDifferentFormats) {
  auto now = system_clock::now();
  auto formatted = format_local_time(now, "%Y-%m-%d %H:%M:%S");
  EXPECT_FALSE(formatted.empty());
}

// ============================================================================
// event_formatter::format (common::event_list)
// ============================================================================
TEST(EventFormatter, FormatEmptyEventsReturnsDefaultMessage) {
  event_formatter formatter("Нет событий");
  event_list events;
  auto result = formatter.format(events);
  EXPECT_EQ(result, "Нет событий");
}

TEST(EventFormatter, FormatSingleEvent) {
  event_formatter formatter("Нет событий");
  auto events = std::vector<event>{make_event("Встреча", 10, 0, 10, 30)};

  auto result = formatter.format(events);
  EXPECT_FALSE(result.empty());

  auto start_time = to_local_str(events[0].start, "%H:%M");
  auto end_time = to_local_str(events[0].end, "%H:%M");
  EXPECT_NE(result.find(start_time), std::string::npos);
  EXPECT_NE(result.find(end_time), std::string::npos);
  EXPECT_NE(result.find("Встреча"), std::string::npos);
}

TEST(EventFormatter, FormatSingleEventWithColon) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event("Созвон: обсуждение", 14, 0, 15, 0)};

  auto result = formatter.format(events);
  EXPECT_NE(result.find("Созвон: обсуждение"), std::string::npos);
}

TEST(EventFormatter, FormatMultipleEvents) {
  event_formatter formatter("Нет событий");
  auto events = std::vector<event>{
      make_event("Первое", 9, 0, 9, 30), make_event("Второе", 10, 0, 10, 30), make_event("Третье", 14, 0, 15, 0)};

  auto result = formatter.format(events);
  EXPECT_NE(result.find("Первое"), std::string::npos);
  EXPECT_NE(result.find("Второе"), std::string::npos);
  EXPECT_NE(result.find("Третье"), std::string::npos);

  auto newline_count = 0u;
  for (char c: result) {
    if (c == '\n')
      ++newline_count;
  }
  EXPECT_EQ(newline_count, 3u);
}

TEST(EventFormatter, FormatEventWithEmptySummary) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event("", 12, 0, 13, 0)};

  auto result = formatter.format(events);
  EXPECT_FALSE(result.empty());
}

TEST(EventFormatter, FormatEventWithLongSummary) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event(std::string(500, 'X'), 8, 0, 9, 0)};

  auto result = formatter.format(events);
  EXPECT_NE(result.find("X"), std::string::npos);
}

TEST(EventFormatter, FormatEventWithLocation) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event("Конференция", 10, 0, 11, 0, "", "Переговорная 1")};

  auto result = formatter.format(events);
  // Location is logged but not included in formatted output
  EXPECT_FALSE(result.empty());
}

TEST(EventFormatter, FormatEventWithDescription) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event("Отчёт", 15, 0, 16, 0, "Подготовить отчёт за Q3")};

  auto result = formatter.format(events);
  EXPECT_FALSE(result.empty());
}

TEST(EventFormatter, FormatEmptyDescription) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event("Заметка", 9, 0, 9, 15, "", "")};

  auto result = formatter.format(events);
  EXPECT_FALSE(result.empty());
}

TEST(EventFormatter, FormatManyEvents) {
  event_formatter formatter;
  std::vector<event> events;
  for (int i = 0; i < 50; ++i) {
    events.push_back(make_event("Событие " + std::to_string(i), i % 24, 0, i % 24, 30));
  }

  auto result = formatter.format(events);
  EXPECT_FALSE(result.empty());
  for (int i = 0; i < 50; ++i) {
    EXPECT_NE(result.find("Событие " + std::to_string(i)), std::string::npos);
  }
}

// ============================================================================
// event_formatter::format (ics::scheduled_event_list)
// ============================================================================
TEST(EventFormatter, FormatScheduledEvent) {
  event_formatter formatter;
  divoomdev::ics::scheduled_event_list scheduled;
  divoomdev::ics::scheduled_event sev;
  sev.summary = "ICS Event";
  sev.start = system_clock::now();
  sev.end = system_clock::now() + std::chrono::hours(1);
  scheduled.push_back(sev);

  auto result = formatter.format(scheduled);
  EXPECT_NE(result.find("ICS Event"), std::string::npos);
}

TEST(EventFormatter, FormatEmptyScheduledEvents) {
  event_formatter formatter("Нет событий");
  divoomdev::ics::scheduled_event_list scheduled;
  auto result = formatter.format(scheduled);
  EXPECT_EQ(result, "Нет событий");
}

TEST(EventFormatter, FormatMultipleScheduledEvents) {
  event_formatter formatter;
  divoomdev::ics::scheduled_event_list scheduled;
  for (int i = 0; i < 3; ++i) {
    divoomdev::ics::scheduled_event sev;
    sev.summary = "Scheduled " + std::to_string(i);
    sev.start = system_clock::now();
    sev.end = system_clock::now() + std::chrono::hours(1);
    scheduled.push_back(sev);
  }

  auto result = formatter.format(scheduled);
  EXPECT_NE(result.find("Scheduled 0"), std::string::npos);
  EXPECT_NE(result.find("Scheduled 1"), std::string::npos);
  EXPECT_NE(result.find("Scheduled 2"), std::string::npos);
}

// ============================================================================
// event_formatter constructor
// ============================================================================
TEST(EventFormatter, DefaultConstructor) {
  event_formatter formatter;
  event_list events;
  auto result = formatter.format(events);
  EXPECT_EQ(result, "Нет событий в календаре");
}

TEST(EventFormatter, CustomDefaultMessage) {
  event_formatter formatter("Доступных событий на сегодня нет");
  event_list events;
  auto result = formatter.format(events);
  EXPECT_EQ(result, "Доступных событий на сегодня нет");
}

// ============================================================================
// Unicode and special characters
// ============================================================================
TEST(EventFormatter, FormatEventWithUnicode) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event("Привет мир 你好世界", 10, 0, 11, 0)};

  auto result = formatter.format(events);
  EXPECT_NE(result.find("Привет"), std::string::npos);
  EXPECT_NE(result.find("你好"), std::string::npos);
}

TEST(EventFormatter, FormatEventWithSpecialChars) {
  event_formatter formatter;
  auto events = std::vector<event>{make_event("<html>&'\"<>", 10, 0, 11, 0)};

  auto result = formatter.format(events);
  EXPECT_NE(result.find("<"), std::string::npos);
}
