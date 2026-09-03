#pragma once
#include <chrono>
#include <string>
#include <vector>

#include "ics/scheduled_event.h"

namespace divoomdev::ics::detail {

/// Основной парсер календаря: unfold -> разбор VEVENT -> раскрытие повторений ->
/// применение оверрайдов -> фильтр по окну -> сортировка.
///
/// Принимает явный момент "now" для детерминированного тестирования.
std::vector<scheduled_event> parse_calendar_at(const std::string& ics_content,
                                               std::chrono::time_point<std::chrono::system_clock> now);

}  // namespace divoomdev::ics::detail