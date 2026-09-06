#pragma once
#include <string>

namespace divoomdev::jira {

struct issue {
  std::string key;       // Ключ задачи, например "PROJ-123"
  std::string link;      // Ссылка на задачу
  std::string summary;   // Название задачи
  std::string status;    // Текущий статус (In Progress, To Do)
  std::string assignee;  // Исполнитель (для проверки)
};
using issues_vec = std::vector<issue>;

}  // namespace divoomdev::jira
