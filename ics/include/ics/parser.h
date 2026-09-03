#pragma once
#include "scheduled_event.h"
#include <string>
#include <vector>

namespace divoomdev::ics {

/// Загружает ICS-файл по URL и возвращает события на ближайшие 48 часов.
/// События со статусом CANCELED исключаются.
std::vector<scheduled_event> fetch_events(const std::string& url);

/// Парсит ICS-контент (строку) и возвращает события на ближайшие 48 часов.
std::vector<scheduled_event> parse_calendar(const std::string& ics_content);

}  // namespace divoomdev::ics
