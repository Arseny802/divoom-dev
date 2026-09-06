#pragma once
#include <string>

#include <nlohmann/json.hpp>

#include "issue.h"

namespace divoomdev::jira {

class client {
 public:
  struct config {
    std::string base_url;   // Например: https://your-company.atlassian.net
    std::string email;      // Ваша почта в Atlassian
    std::string api_token;  // Сгенерированный API Token
    int timeout_seconds = 30;
  };

  explicit client(const config& config);

  /**
   * @brief Получает задачи, назначенные на пользователя, которые не закрыты.
   * @param jql Дополнительный фильтр JQL (например, "project = MYPROJ").
   *            Если пусто, ищет по всем проектам.
   */
  issues_vec GetActiveUserIssues(const std::string& extra_filter = "", const std::string& order_by = "");

 private:
  config config_;
  issues_vec ParseSearchResults(const nlohmann::json& json_response);
};

}  // namespace divoomdev::jira
