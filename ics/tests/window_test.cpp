#include "gtest/gtest.h"

#include "test_helpers.h"
#include "window.h"

using namespace divoomdev::ics::detail;

TEST(Window, StartAtLocalMidnight) {
  auto now = ics_test::wall_clock(2026, 9, 4, 15, 30, 45);
  auto win = compute_window(now);
  auto midnight = ics_test::wall_clock(2026, 9, 4, 0, 0, 0);
  EXPECT_EQ(win.start, midnight);
  EXPECT_EQ(win.deadline, now + std::chrono::hours(48));
}

TEST(Window, InWindowBounds) {
  auto now = ics_test::wall_clock(2026, 9, 4, 15, 0, 0);
  auto win = compute_window(now);
  EXPECT_TRUE(in_window(win.start, win));
  EXPECT_TRUE(in_window(win.deadline, win));
  EXPECT_TRUE(in_window(now, win));
  EXPECT_FALSE(in_window(win.start - std::chrono::seconds(1), win));
  EXPECT_FALSE(in_window(win.deadline + std::chrono::seconds(1), win));
}

TEST(Window, ZeroTimePointNotInWindow) {
  auto now = ics_test::wall_clock(2026, 9, 4, 15, 0, 0);
  auto win = compute_window(now);
  std::chrono::time_point<std::chrono::system_clock> zero;
  EXPECT_FALSE(in_window(zero, win));
}