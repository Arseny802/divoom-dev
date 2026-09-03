#include "ics/parser.h"
#include "ics/scheduled_event.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

std::string format_time(std::chrono::time_point<std::chrono::system_clock> tp) {
  auto t = std::chrono::system_clock::to_time_t(tp);
  struct tm tm;
  localtime_s(&tm, &t);
  char buf[64];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
  return std::string(buf);
}

int main() {
  // Читаем файл календаря
  std::ifstream file("ics/calendar.ics");
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open ics/calendar.ics" << std::endl;
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string ics_content = buffer.str();

  std::cout << "=== Parsing ICS calendar ===" << std::endl;
  std::cout << "File size: " << ics_content.size() << " bytes" << std::endl;

  // Парсим календарь
  auto events = divoomdev::ics::parse_calendar(ics_content);

  std::cout << "\n=== Found " << events.size() << " events in next 48 hours ===" << std::endl;

  auto now = std::chrono::system_clock::now();
  std::cout << "Current time: " << format_time(now) << std::endl;

  auto deadline = now + std::chrono::hours(48);
  std::cout << "Deadline: " << format_time(deadline) << std::endl;

  if (events.empty()) {
    std::cout << "\nWARNING: No events found! This might be a bug." << std::endl;
    std::cout << "\n=== Debug: Checking all events (without 48h filter) ===" << std::endl;

    // Попробуем распарсить без фильтра 48 часов
    // (для этого нужно временно изменить логику)
    std::cout << "All parsed events have these dates:" << std::endl;
    // Временно выключим фильтр
    // ...
  }

  for (size_t i = 0; i < events.size(); ++i) {
    const auto& ev = events[i];
    std::cout << "\n--- Event " << (i + 1) << " ---" << std::endl;
    std::cout << "  Summary: " << ev.summary << std::endl;
    std::cout << "  Start: " << format_time(ev.start) << std::endl;
    std::cout << "  End: " << format_time(ev.end) << std::endl;
    std::cout << "  Location: " << ev.location << std::endl;
    std::cout << "  Description: " << ev.description << std::endl;
    std::cout << "  UID: " << ev.uid << std::endl;
    std::cout << "  Repeat: " << static_cast<int>(ev.repeat) << std::endl;
    std::cout << "  Byday mask: " << static_cast<int>(ev.byday_mask) << std::endl;
    std::cout << "  Exdates count: " << ev.exdates.size() << std::endl;
    std::cout << "  Alarms: " << ev.alarms.size() << std::endl;
  }

  return 0;
}
