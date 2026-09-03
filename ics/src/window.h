#pragma once
#include <chrono>

namespace divoomdev::ics::detail {

/// Окно выборки событий: от начала текущего дня (локально) до now+48ч.
/// Нижняя граница — начало дня, чтобы не терять события, уже начавшиеся сегодня.
struct time_window {
  std::chrono::time_point<std::chrono::system_clock> start;
  std::chrono::time_point<std::chrono::system_clock> deadline;
};

/// Строит окно относительно заданного момента (для тестирования передаётся now).
time_window compute_window(std::chrono::time_point<std::chrono::system_clock> now);

/// Проверяет, попадает ли момент в окно (включая нижнюю границу).
bool in_window(std::chrono::time_point<std::chrono::system_clock> tp, const time_window& win);

}  // namespace divoomdev::ics::detail