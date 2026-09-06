#include "jira/client.h"
#include <cpr/cpr.h>

namespace divoomdev::jira {

client::client(const config& config): config_(config) { }

issues_vec client::ParseSearchResults(const nlohmann::json& json_response) {
  issues_vec issues;
  try {
    for (const auto& issue_json: json_response["issues"]) {
      issue issue;
      issue.key = issue_json.value("key", "N/A");
      issue.link = issue_json.value("self", "N/A");

      const auto& fields = issue_json["fields"];
      issue.summary = fields.value("summary", "No Summary");

      if (fields.contains("status") && fields["status"].contains("name")) {
        issue.status = fields["status"]["name"].get<std::string>();
      }

      if (fields.contains("assignee") && fields["assignee"].contains("emailAddress")) {
        issue.assignee = fields["assignee"]["emailAddress"].get<std::string>();
      } else {
        issue.assignee = "Unassigned";
      }

      issues.push_back(issue);
    }
  } catch (const nlohmann::json::exception& e) {
    log()->error("JSON parsing error in Jira response: {}", e.what());
  }
  return issues;
}

issues_vec client::GetActiveUserIssues(const std::string& extra_filter, const std::string& order_by) {
  std::string base_jql = "assignee=currentUser() AND Спринт in openSprints()";
  if (!extra_filter.empty()) {
    base_jql += " AND (" + extra_filter + ")";
  }
  if (!order_by.empty()) {
    base_jql += " ORDER BY " + order_by;
  }

  cpr::Url url{config_.base_url + "/rest/api/latest/search"};
  cpr::Parameters params{{"jql", base_jql}, {"maxResults", "50"}, {"fields", "summary,status,assignee"}};
  cpr::SslOptions ssl_opt{.verify_host = false, .verify_peer = false};  // NOLINT
  cpr::Timeout timeout{config_.timeout_seconds * 1000};
  cpr::Header headers{
      {"Content-Type", "application/json"},
      {"Accept", "application/json"},
      {"Authorization", "Bearer " + config_.api_token},
      {"X-Atlassian-Token", "no-check"},
  };

  auto response = cpr::Get(url, headers, params, ssl_opt, timeout);
  if (response.status_code != 200) {
    log()->error("Jira request failed. Status: {}", response.status_code);
    log()->error("Body: {}", response.text.substr(0, 500));
    return {};
  }

  if (response.text.find("You must log in") != std::string::npos) {
    log()->error("[CRITICAL] Server rejected Authorization header.");
    log()->error("Check that Email and Token are correct for URL: {}", config_.base_url);
    return {};
  }

  try {
    auto json_root = nlohmann::json::parse(response.text);
    return ParseSearchResults(json_root);
  } catch (const nlohmann::json::exception& e) {
    log()->error("Failed to parse JSON from Jira: {}", response.text);
    return {};
  }
}
}  // namespace divoomdev::jira
