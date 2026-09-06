#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "common/event.h"
#include "ics/scheduled_event.h"

namespace divoomdev::service::core {

/// Форматирует момент времени в локальную строку по заданному формату.
std::string format_local_time(std::chrono::time_point<std::chrono::system_clock> tp, const char* fmt);

/// Форматирует список событий календаря в текст для отображения.
class event_formatter {
 public:
  using event_list = std::vector<common::event>;

  event_formatter() noexcept;
  ~event_formatter() noexcept;
  event_formatter(std::string default_message) noexcept;

  /// Форматирует события в текст с интервалами и суммариями.
  std::string format(const event_list& events);
  std::string format(const ics::scheduled_event_list& events);

 private:
  const std::string default_message_ = " - Нет событий в календаре";
};

}  // namespace divoomdev::service::core
