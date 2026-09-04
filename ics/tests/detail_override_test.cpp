#include "gtest/gtest.h"

#include <vector>

#include "detail/override_applier.h"
#include "detail/raw_event.h"
#include "detail/window.h"
#include "ics/settings.h"
#include "test_helpers.h"

using namespace divoomdev::common;
using namespace divoomdev::ics;
using namespace divoomdev::ics::detail;
using tp = std::chrono::time_point<std::chrono::system_clock>;

namespace {
raw_event make_raw(const std::string& uid, tp start, bool has_rid, const std::string& rid = "", bool canceled = false) {
  raw_event r;
  r.ev.uid = uid;
  r.ev.start = start;
  if (canceled)
    r.ev.status = event_status::CANCELED;
  r.has_recurrence_id = has_rid;
  r.recurrence_id = rid;
  return r;
}
}  // namespace

TEST(OverrideIndex, GroupsByUid) {
  auto r1 = make_raw("uidA", ics_test::wall_clock(2026, 9, 4, 10, 0, 0), true, "20260904T100000");
  auto r2 = make_raw("uidB", ics_test::wall_clock(2026, 9, 4, 11, 0, 0), true, "20260904T110000");
  auto r3 = make_raw("uidA", ics_test::wall_clock(2026, 9, 4, 12, 0, 0), false);
  std::vector<raw_event> raws{r1, r2, r3};

  auto idx = override_applier::build_index(raws);
  ASSERT_EQ(idx.count("uidA"), 1u);
  ASSERT_EQ(idx.count("uidB"), 1u);
  EXPECT_EQ(idx.at("uidA")[0]->recurrence_id, "20260904T100000");
}

TEST(IsOverridden, MatchesInstanceKey) {
  auto r = make_raw("uidA", ics_test::wall_clock(2026, 9, 4, 10, 0, 0), true, "20260904T100000");
  auto idx = override_applier::build_index(std::vector<raw_event>{r});
  EXPECT_TRUE(override_applier::is_overridden(idx, "uidA", "20260904T100000"));
  EXPECT_FALSE(override_applier::is_overridden(idx, "uidA", "20260904T090000"));
  EXPECT_FALSE(override_applier::is_overridden(idx, "uidB", "20260904T100000"));
}

TEST(AddInWindowOverrides, AddsOnlyNonCanceledInsideWindow) {
  auto win = compute_window(ics_test::wall_clock(2026, 9, 4, 12, 0, 0), {});
  auto moved = make_raw("uidA", ics_test::wall_clock(2026, 9, 4, 15, 0, 0), true, "20260904T100000");
  auto canceled = make_raw("uidB", ics_test::wall_clock(2026, 9, 4, 16, 0, 0), true, "20260904T110000", true);
  auto outside = make_raw("uidC", ics_test::wall_clock(2027, 1, 1, 10, 0, 0), true, "20270101T100000");
  std::vector<raw_event> raws{moved, canceled, outside};

  auto idx = override_applier::build_index(raws);
  std::vector<scheduled_event> result;
  override_applier::add_in_window_overrides(idx, raws, win, result);

  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0].uid, "uidA");
}