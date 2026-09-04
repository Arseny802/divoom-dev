#pragma once
#include <map>
#include <string>
#include <vector>

#include "raw_event.h"
#include "window.h"

namespace divoomdev::ics::detail {

/// Обрабатывает RECURRENCE-ID-оверрайды (перенесённые/отменённые вхождения серии).
///
/// Отдельная ответственность: связывает оверрайды с мастер-событием по UID,
/// исключает переопределённые вхождения из основной серии и добавляет
/// неотменённые оверрайды как самостоятельные события (без дубликатов).
class override_applier {
 public:
  /// Индекс оверрайдов по UID.
  using index = std::map<std::string, std::vector<const raw_event*>>;

  /// Строит индекс из всех сырых событий.
  static index build_index(const std::vector<raw_event>& raws);

  /// Есть ли оверрайд для вхождения мастер-события с данным ключом.
  static bool is_overridden(const index& idx, const std::string& uid, const std::string& instance_key);

  /// Добавляет в result неотменённые оверрайды, попавшие в окно.
  static void add_in_window_overrides(const index& idx,
                                      const std::vector<raw_event>& raws,
                                      const time_window& win,
                                      std::vector<scheduled_event>& result);
};

}  // namespace divoomdev::ics::detail