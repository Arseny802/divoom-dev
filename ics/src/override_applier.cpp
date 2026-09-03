#include "override_applier.h"

namespace divoomdev::ics::detail {

override_applier::index override_applier::build_index(const std::vector<raw_event>& raws) {
  index idx;
  for (const auto& r: raws) {
    if (r.has_recurrence_id)
      idx[r.ev.uid].push_back(&r);
  }
  return idx;
}

bool override_applier::is_overridden(const index& idx, const std::string& uid, const std::string& instance_key) {
  auto it = idx.find(uid);
  if (it == idx.end())
    return false;
  for (const auto* ov: it->second) {
    if (ov->recurrence_id == instance_key)
      return true;
  }
  return false;
}

void override_applier::add_in_window_overrides(const index&,
                                               const std::vector<raw_event>& raws,
                                               const time_window& win,
                                               std::vector<scheduled_event>& result) {
  for (const auto& r: raws) {
    if (!r.has_recurrence_id || r.ev.status == common::event_status::CANCELED)
      continue;
    if (in_window(r.ev.start, win))
      result.push_back(r.ev);
  }
}

}  // namespace divoomdev::ics::detail