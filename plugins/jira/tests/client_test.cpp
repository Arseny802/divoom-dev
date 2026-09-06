#include "gtest/gtest.h"

#include <string>

#include "jira/client.h"

using namespace divoomdev::jira;

// ============================================================================
// config — структура конфигурации
// ============================================================================

TEST(JiraClientConfigTest, DefaultTimeoutSecondsIs30) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "token123";

    EXPECT_EQ(cfg.timeout_seconds, 30);
}

TEST(JiraClientConfigTest, CustomTimeoutSeconds) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "token123";
    cfg.timeout_seconds = 60;

    EXPECT_EQ(cfg.timeout_seconds, 60);
}

TEST(JiraClientConfigTest, ConfigWithAllFields) {
    client::config cfg;
    cfg.base_url = "https://mycompany.atlassian.net";
    cfg.email = "test@mycompany.com";
    cfg.api_token = "secret-api-token";
    cfg.timeout_seconds = 10;

    EXPECT_EQ(cfg.base_url, "https://mycompany.atlassian.net");
    EXPECT_EQ(cfg.email, "test@mycompany.com");
    EXPECT_EQ(cfg.api_token, "secret-api-token");
    EXPECT_EQ(cfg.timeout_seconds, 10);
}

TEST(JiraClientConfigTest, ConfigWithEmptyFields) {
    client::config cfg;
    cfg.timeout_seconds = 15;

    EXPECT_TRUE(cfg.base_url.empty());
    EXPECT_TRUE(cfg.email.empty());
    EXPECT_TRUE(cfg.api_token.empty());
    EXPECT_EQ(cfg.timeout_seconds, 15);
}

// ============================================================================
// client — конструктор
// ============================================================================

TEST(JiraClientTest, ConstructorWithValidConfigDoesNotThrow) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "token123";
    cfg.timeout_seconds = 30;

    EXPECT_NO_THROW(client c(cfg));
}

TEST(JiraClientTest, ConstructorWithMinimalConfigDoesNotThrow) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "token";

    EXPECT_NO_THROW(client c(cfg));
}

TEST(JiraClientTest, MultipleClientsWithDifferentConfig) {
    client::config cfg1;
    cfg1.base_url = "https://company1.atlassian.net";
    cfg1.email = "user1@example.com";
    cfg1.api_token = "token1";

    client::config cfg2;
    cfg2.base_url = "https://company2.atlassian.net";
    cfg2.email = "user2@example.com";
    cfg2.api_token = "token2";

    EXPECT_NO_THROW(client c1(cfg1));
    EXPECT_NO_THROW(client c2(cfg2));
}

// ============================================================================
// GetActiveUserIssues — интеграционные тесты
// Требуют запущенный Jira-сервер с валидными учётными данными.
// По умолчанию пропущены (DISABLED_).
// ============================================================================

TEST(JiraClientTest, DISABLED_GetActiveUserIssuesWithValidConfigDoesNotThrow) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "valid-token";
    cfg.timeout_seconds = 30;

    client c(cfg);
    EXPECT_NO_THROW(c.GetActiveUserIssues());
}

TEST(JiraClientTest, DISABLED_GetActiveUserIssuesWithExtraFilterDoesNotThrow) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "valid-token";

    client c(cfg);
    EXPECT_NO_THROW(c.GetActiveUserIssues("project = MYPROJ"));
}

TEST(JiraClientTest, DISABLED_GetActiveUserIssuesWithOrderByDoesNotThrow) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "valid-token";

    client c(cfg);
    EXPECT_NO_THROW(c.GetActiveUserIssues("", "updated DESC"));
}

TEST(JiraClientTest, DISABLED_GetActiveUserIssuesReturnsEmptyWhenNoIssues) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "valid-token";

    client c(cfg);
    auto issues = c.GetActiveUserIssues("assignee = nonexistent_user");
    EXPECT_EQ(issues.size(), 0u);
}

// ============================================================================
// Issue — тесты структуры данных
// ============================================================================

TEST(JiraIssueTest, IssueWithAllFields) {
    issue iss;
    iss.key = "PROJ-123";
    iss.link = "https://example.atlassian.net/browse/PROJ-123";
    iss.summary = "Fix login bug";
    iss.status = "In Progress";
    iss.assignee = "John Doe";

    EXPECT_EQ(iss.key, "PROJ-123");
    EXPECT_EQ(iss.link, "https://example.atlassian.net/browse/PROJ-123");
    EXPECT_EQ(iss.summary, "Fix login bug");
    EXPECT_EQ(iss.status, "In Progress");
    EXPECT_EQ(iss.assignee, "John Doe");
}

TEST(JiraIssueTest, IssueWithEmptyFields) {
    issue iss;

    EXPECT_TRUE(iss.key.empty());
    EXPECT_TRUE(iss.link.empty());
    EXPECT_TRUE(iss.summary.empty());
    EXPECT_TRUE(iss.status.empty());
    EXPECT_TRUE(iss.assignee.empty());
}

TEST(JiraIssueTest, IssuesVecEmptyByDefault) {
    issues_vec issues;
    EXPECT_TRUE(issues.empty());
    EXPECT_EQ(issues.size(), 0u);
}

TEST(JiraIssueTest, IssuesVecWithMultipleIssues) {
    issues_vec issues;

    issue i1;
    i1.key = "PROJ-1";
    i1.summary = "Task 1";
    i1.status = "To Do";
    issues.push_back(i1);

    issue i2;
    i2.key = "PROJ-2";
    i2.summary = "Task 2";
    i2.status = "In Progress";
    issues.push_back(i2);

    EXPECT_EQ(issues.size(), 2u);
    EXPECT_EQ(issues[0].key, "PROJ-1");
    EXPECT_EQ(issues[1].key, "PROJ-2");
}

// ============================================================================
// Edge cases — краевые значения
// ============================================================================

TEST(JiraClientConfigTest, ConfigWithVeryLongValues) {
    client::config cfg;
    cfg.base_url = std::string(255, 'a') + ".atlassian.net";
    cfg.email = std::string(255, 'x') + "@example.com";
    cfg.api_token = std::string(512, 'y');

    EXPECT_NO_THROW(client c(cfg));
}

TEST(JiraClientConfigTest, ConfigWithSpecialCharactersInEmail) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user+tag@example.com";
    cfg.api_token = "token-with-dashes";

    EXPECT_NO_THROW(client c(cfg));
}

TEST(JiraClientConfigTest, ConfigWithZeroTimeout) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "token";
    cfg.timeout_seconds = 0;

    EXPECT_NO_THROW(client c(cfg));
}

TEST(JiraClientConfigTest, ConfigWithNegativeTimeout) {
    client::config cfg;
    cfg.base_url = "https://example.atlassian.net";
    cfg.email = "user@example.com";
    cfg.api_token = "token";
    cfg.timeout_seconds = -1;

    EXPECT_NO_THROW(client c(cfg));
}
