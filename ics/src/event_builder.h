#pragma once
#include <string>
#include <vector>

#include "raw_event.h"
#include "timezone.h"

namespace divoomdev::ics::detail {

/// Строитель (Builder) события: превращает строки VEVENT-блока в raw_event.
///
/// Свойства применяются через реестр обработчиков (паттерн "Фабрика/Реестр"):
/// каждому имени свойства соответствует свой обработчик, поэтому добавление
/// нового свойства не меняет существующий код (принцип открытости/закрытости).
class event_builder {
 public:
  explicit event_builder(const timezone_resolver& tz);

  /// Собирает raw_event из строк VEVENT-блока.
  /// alarm_lines — содержимое вложенных блоков VALARM (присоединяется к событию).
  raw_event build(const std::vector<std::string>& content_lines, const std::vector<std::string>& alarm_lines) const;

 private:
  const timezone_resolver& tz_;
};

/// Парсит VALARM и добавляет alarm в событие.
void parse_alarm(const std::vector<std::string>& lines, scheduled_event& ev);

}  // namespace divoomdev::ics::detail