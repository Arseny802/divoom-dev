#include "gtest/gtest.h"

#include "text_utils.h"

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
  EXPECT_EQ(to_upper(""), "");
}

TEST(TextUtils, Unescape) {
  EXPECT_EQ(unescape("Hello\\, world"), "Hello, world");
  EXPECT_EQ(unescape("a\\;b"), "a;b");
  EXPECT_EQ(unescape("back\\\\slash"), "back\\slash");
  EXPECT_EQ(unescape("line1\\nline2"), "line1\nline2");
  EXPECT_EQ(unescape("line\\Nline"), "line\nline");
  EXPECT_EQ(unescape("plain text"), "plain text");
  EXPECT_EQ(unescape("trailing\\"), "trailing\\");  // одинокий backslash не ломается
}

TEST(TextUtils, UnfoldJoinsContinuationLines) {
  const std::string folded = "BEGIN:VEVENT\n"
                             "SUMMARY:Daily M6 (команда)\n"
                             "  - длинное продолжение\n"
                             "\tещё таб-продолжение\n"
                             "UID:abc\n"
                             "END:VEVENT\n";
  std::string out = unfold(folded);
  // Продолжения склеиваются напрямую (снимается ровно один ведущий пробел/таб).
  EXPECT_NE(out.find("SUMMARY:Daily M6 (команда) - длинное продолжениеещё таб-продолжение"), std::string::npos);
  EXPECT_NE(out.find("UID:abc"), std::string::npos);
}

TEST(TextUtils, UnfoldHandlesCrLfAndBlankLines) {
  const std::string folded = "A:1\r\n"
                             " B:2\r\n"
                             "\r\n"
                             "C:3\r\n";
  std::string out = unfold(folded);
  EXPECT_NE(out.find("A:1B:2"), std::string::npos);  // B:2 — продолжение A:1
  EXPECT_NE(out.find("C:3"), std::string::npos);
}

TEST(TextUtils, UnfoldKeepsPlainLines) {
  std::string out = unfold("A:1\nB:2\n");
  EXPECT_NE(out.find("A:1"), std::string::npos);
  EXPECT_NE(out.find("B:2"), std::string::npos);
}

TEST(TextUtils, SplitIcsLine) {
  auto [key, params, value] = split_ics_line("DTSTART;TZID=Europe/Moscow:20231202T120000");
  EXPECT_EQ(key, "DTSTART");
  EXPECT_EQ(params, "TZID=Europe/Moscow");
  EXPECT_EQ(value, "20231202T120000");

  auto [k2, p2, v2] = split_ics_line("SUMMARY:Hello World");
  EXPECT_EQ(k2, "SUMMARY");
  EXPECT_EQ(p2, "");
  EXPECT_EQ(v2, "Hello World");

  auto [k3, p3, v3] = split_ics_line("no-colon");
  EXPECT_EQ(k3, "");
  EXPECT_EQ(v3, "");
  (void)p3;
}

TEST(TextUtils, SplitIcsLineKeyUpper) {
  auto [key, params, value] = split_ics_line("dtstart;tzid=x:val");
  EXPECT_EQ(key, "DTSTART");
  EXPECT_EQ(value, "val");
  EXPECT_EQ(params, "tzid=x");
}