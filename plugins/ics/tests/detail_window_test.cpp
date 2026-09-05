#include "gtest/gtest.h"

#include "detail/window.h"
#include "ics/settings.h"
#include "test_helpers.h"

using namespace divoomdev::ics;
using namespace divoomdev::ics::detail;

TEST(Window, DefaultSettingsStartAtLocalMidnight) {
  auto now = ics_test::wall_clock(2026, 9, 4, 15, 30, 45);
  auto win = compute_window(now, {});
  EXPECT_EQ(win.start, ics_test::wall_clock(2026, 9, 4, 0, 0, 0));
  EXPECT_EQ(win.deadline, now + std::chrono::hours(48));
}

TEST(Window, CustomHorizon) {
  calendar_settings settings;
  settings.horizon = std::chrono::hours(24);
  auto now = ics_test::wall_clock(2026, 9, 4, 10, 0, 0);
  auto win = compute_window(now, settings);
  EXPECT_EQ(win.deadline, now + std::chrono::hours(24));
}

TEST(Window, WindowFromNowWhenDisabled) {
  calendar_settings settings;
  settings.window_from_day_start = false;
  auto now = ics_test::wall_clock(2026, 9, 4, 10, 0, 0);
  auto win = compute_window(now, settings);
  EXPECT_EQ(win.start, now);
}

TEST(Window, InWindowBoundsInclusive) {
  auto now = ics_test::wall_clock(2026, 9, 4, 15, 0, 0);
  auto win = compute_window(now, {});
  EXPECT_TRUE(in_window(win.start, win));
  EXPECT_TRUE(in_window(win.deadline, win));
  EXPECT_TRUE(in_window(now, win));
  EXPECT_FALSE(in_window(win.start - std::chrono::seconds(1), win));
  EXPECT_FALSE(in_window(win.deadline + std::chrono::seconds(1), win));
}

TEST(Window, ZeroTimePointNotInWindow) {
  auto win = compute_window(ics_test::wall_clock(2026, 9, 4, 15, 0, 0), {});
  std::chrono::time_point<std::chrono::system_clock> zero;
  EXPECT_FALSE(in_window(zero, win));
}