#pragma once
#include <string>

namespace divoomdev::storage {

/// Учётная запись онлайн-сервиса.
///
/// `service` — имя сервиса (например, "divoom"). Поле позволяет хранить
/// произвольное количество сервисов без изменения схемы/интерфейса.
struct account {
  std::string service;
  std::string login;
  std::string password;
};

/// ICS-календарь: URL источника и параметры загрузки.
struct calendar_source {
  std::string url;
  /// Приоритет загрузки. Большее значение = выше приоритет.
  int priority = 0;
  bool enabled = true;
};

#ifdef NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(account, service, login, password);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(calendar_source, url, priority, enabled);
#endif

}  // namespace divoomdev::storage