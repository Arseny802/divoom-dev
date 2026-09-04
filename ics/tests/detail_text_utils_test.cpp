#include "gtest/gtest.h"

#include "detail/text_utils.h"

using namespace divoomdev::ics::detail;

TEST(TextUtils, Trim) {
  EXPECT_EQ(trim(""), "");
  EXPECT_EQ(trim("   "), "");
  EXPECT_EQ(trim("  hello  "), "hello");
  EXPECT_EQ(trim("\t\n hello \r\n"), "hello");
  EXPECT_EQ(trim("no-trim"), "no-trim");
}

TEST(TextUtils, ToUpper) {
  EXPECT_EQ(to_upper("FREQ=WEEKLY;BYDAY=MO,FR"), "FREQ=WEEKLY;BYDAY=MO,FR");
  EXPECT_EQ(to_upper("AbC"), "ABC");
}

TEST(TextUtils, UnescapeHandlesSupportedEscapes) {
  EXPECT_EQ(unescape("Hello\\, world"), "Hello, world");
  EXPECT_EQ(unescape("a\\;b"), "a;b");
  EXPECT_EQ(unescape("back\\\\slash"), "back\\slash");
  EXPECT_EQ(unescape("line1\\nline2"), "line1\nline2");
  EXPECT_EQ(unescape("line\\Nline"), "line\nline");
  EXPECT_EQ(unescape("plain text"), "plain text");
  EXPECT_EQ(unescape("trailing\\"), "trailing\\");  // lone backslash is safe
}

TEST(TextUtils, UnfoldJoinsContinuationLines) {
  const std::string folded = "BEGIN:VEVENT\n"
                             "SUMMARY:Team Sync\n"
                             "  - folded line one\n"
                             "\tsecond continuation\n"
                             "UID:abc\n"
                             "END:VEVENT\n";
  std::string out = unfold(folded);
  // Continuations join directly; exactly one leading space/tab is removed.
  EXPECT_NE(out.find("SUMMARY:Team Sync - folded line onesecond continuation"), std::string::npos);
  EXPECT_NE(out.find("UID:abc"), std::string::npos);
}

TEST(TextUtils, UnfoldHandlesCrLfAndBlankLines) {
  const std::string folded = "A:1\r\n"
                             " B:2\r\n"
                             "\r\n"
                             "C:3\r\n";
  std::string out = unfold(folded);
  EXPECT_NE(out.find("A:1B:2"), std::string::npos);
  EXPECT_NE(out.find("C:3"), std::string::npos);
}

TEST(TextUtils, SplitIcsLineKeyParamsValue) {
  auto [key, params, value] = split_ics_line("DTSTART;TZID=Europe/Moscow:20231202T120000");
  EXPECT_EQ(key, "DTSTART");
  EXPECT_EQ(params, "TZID=Europe/Moscow");
  EXPECT_EQ(value, "20231202T120000");
}

TEST(TextUtils, SplitIcsLineNoParamsAndKeyUpper) {
  auto [k2, p2, v2] = split_ics_line("summary:Hello World");
  EXPECT_EQ(k2, "SUMMARY");
  EXPECT_EQ(p2, "");
  EXPECT_EQ(v2, "Hello World");
}