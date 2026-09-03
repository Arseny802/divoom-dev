#pragma once
#include <chrono>
#include <set>
#include <string>
#include <vector>

#include "ics/scheduled_event.h"

namespace divoomdev::ics::detail {

/// Разобранная RRULE.
struct rrule_data {
  enum class freq_t { DAILY, WEEKLY, MONTHLY, YEARLY };
  freq_t freq = freq_t::DAILY;
  int interval = 1;
  std::chrono::time_point<std::chrono::system_clock> until;  // epoch = без ограничения
  std::set<int> byday;                                       // дни недели (1=Пн, 7=Вс)
};

/// Парсит строку RRULE в структуру.
rrule_data parse_rrule_data(const std::string& rrule);

/// Проверяет, исключено ли данное вхождение EXDATE (локальное время "%Y%m%dT%H%M%S").
bool is_excluded(std::chrono::time_point<std::chrono::system_clock> instance_start,
                 const std::set<std::string>& exdates);

/// Наполняет scheduled_event метаданными повторения (repeat, интервал, byday, конец).
void apply_repeat_metadata(scheduled_event& ev, const rrule_data& rule);

/// Стратегия раскрытия повторяющегося события в окне [window_start, deadline].
/// Каждый тип FREQ реализуется отдельной стратегией (см. паттерн "Стратегия").
class recurrence_expander {
 public:
  virtual ~recurrence_expander() = default;

  virtual void expand(const scheduled_event& base,
                      const rrule_data& rule,
                      std::chrono::time_point<std::chrono::system_clock> window_start,
                      std::chrono::time_point<std::chrono::system_clock> deadline,
                      const std::set<std::string>& exdates,
                      std::vector<scheduled_event>& output) const = 0;

  /// Фабрика: возвращает стратегию для заданной частоты.
  static const recurrence_expander& for_freq(rrule_data::freq_t freq);
};

}  // namespace divoomdev::ics::detail