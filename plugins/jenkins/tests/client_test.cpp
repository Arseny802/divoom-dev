#include "gtest/gtest.h"

#include <limits>
#include <string>

#include "jenkins/client.h"

using namespace divoomdev::jenkins;

// ============================================================================
// config — структура конфигурации
// ============================================================================

TEST(JenkinsClientConfigTest, DefaultTimeoutSecondsIs30) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "token123";

  EXPECT_EQ(cfg.timeout_seconds, 30);
}

TEST(JenkinsClientConfigTest, CustomTimeoutSeconds) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "token123";
  cfg.timeout_seconds = 60;

  EXPECT_EQ(cfg.timeout_seconds, 60);
}

TEST(JenkinsClientConfigTest, ConfigWithAllFields) {
  client::config cfg;
  cfg.base_url = "https://myjenkins.example.com:8080";
  cfg.username = "jenkins-admin";
  cfg.api_token = "secret-api-token-xyz";
  cfg.timeout_seconds = 10;

  EXPECT_EQ(cfg.base_url, "https://myjenkins.example.com:8080");
  EXPECT_EQ(cfg.username, "jenkins-admin");
  EXPECT_EQ(cfg.api_token, "secret-api-token-xyz");
  EXPECT_EQ(cfg.timeout_seconds, 10);
}

TEST(JenkinsClientConfigTest, ConfigWithEmptyFields) {
  client::config cfg;
  cfg.timeout_seconds = 15;

  EXPECT_TRUE(cfg.base_url.empty());
  EXPECT_TRUE(cfg.username.empty());
  EXPECT_TRUE(cfg.api_token.empty());
  EXPECT_EQ(cfg.timeout_seconds, 15);
}

// ============================================================================
// client — конструктор
// ============================================================================

TEST(JenkinsClientTest, ConstructorWithValidConfigDoesNotThrow) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "token123";
  cfg.timeout_seconds = 30;

  EXPECT_NO_THROW(client c(cfg));
}

TEST(JenkinsClientTest, ConstructorWithMinimalConfigDoesNotThrow) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "token";

  EXPECT_NO_THROW(client c(cfg));
}

TEST(JenkinsClientTest, ConstructorIsExplicit) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "token";

  // Явное создание — должно компилироваться
  client c(cfg);
  EXPECT_NO_THROW(c.GetConfig());
}

TEST(JenkinsClientTest, MultipleClientsWithDifferentConfig) {
  client::config cfg1;
  cfg1.base_url = "https://jenkins1.example.com";
  cfg1.username = "user1";
  cfg1.api_token = "token1";

  client::config cfg2;
  cfg2.base_url = "https://jenkins2.example.com";
  cfg2.username = "user2";
  cfg2.api_token = "token2";

  EXPECT_NO_THROW(client c1(cfg1));
  EXPECT_NO_THROW(client c2(cfg2));
}

// ============================================================================
// GetConfig
// ============================================================================

TEST(JenkinsClientTest, GetConfigReturnsOriginalConfig) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com:8080";
  cfg.username = "admin";
  cfg.api_token = "my-token";
  cfg.timeout_seconds = 45;

  client c(cfg);
  const auto& retrieved = c.GetConfig();

  EXPECT_EQ(retrieved.base_url, "https://jenkins.example.com:8080");
  EXPECT_EQ(retrieved.username, "admin");
  EXPECT_EQ(retrieved.api_token, "my-token");
  EXPECT_EQ(retrieved.timeout_seconds, 45);
}

TEST(JenkinsClientTest, GetConfigReturnsReference) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "token";

  client c(cfg);
  const auto& retrieved = c.GetConfig();

  // GetConfig возвращает const reference — модификация через cfg не влияет
  EXPECT_EQ(retrieved.base_url, cfg.base_url);
}

// ============================================================================
// GetUserBuilds — интеграционные тесты
// Требуют запущенный Jenkins-сервер с валидными учётными данными.
// По умолчанию пропущены (DISABLED_).
// ============================================================================

TEST(JenkinsClientTest, DISABLED_GetUserBuildsWithValidConfigDoesNotThrow) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "valid-token";
  cfg.timeout_seconds = 30;

  client c(cfg);
  EXPECT_NO_THROW(c.GetUserBuilds("test-job", "admin"));
}

TEST(JenkinsClientTest, DISABLED_GetUserBuildsReturnsVector) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "valid-token";

  client c(cfg);
  auto builds = c.GetUserBuilds("test-job", "admin");
  EXPECT_NE(builds.size(), 0u);
}

TEST(JenkinsClientTest, DISABLED_GetUserBuildsWithCustomCount) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "valid-token";

  client c(cfg);
  auto builds = c.GetUserBuilds("test-job", "admin", 10);
  EXPECT_LE(builds.size(), 10u);
}

TEST(JenkinsClientTest, DISABLED_GetUserBuildsNonExistentJobReturnsEmpty) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "valid-token";

  client c(cfg);
  auto builds = c.GetUserBuilds("non-existent-job", "admin");
  EXPECT_EQ(builds.size(), 0u);
}

// ============================================================================
// build_info — тесты структуры данных
// ============================================================================

TEST(JenkinsBuildInfoTest, BuildInfoWithAllFields) {
  build_info info;
  info.number = 42;
  info.status = "SUCCESS";
  info.result = "SUCCESS";
  info.building = false;
  info.timestamp = 1700000000;

  EXPECT_EQ(info.number, 42);
  EXPECT_EQ(info.status, "SUCCESS");
  EXPECT_EQ(info.result, "SUCCESS");
  EXPECT_FALSE(info.building);
  EXPECT_EQ(info.timestamp, 1700000000);
}

TEST(JenkinsBuildInfoTest, BuildInfoWithEmptyFields) {
  build_info info{};

  EXPECT_EQ(info.number, 0);
  EXPECT_TRUE(info.status.empty());
  EXPECT_TRUE(info.result.empty());
  EXPECT_FALSE(info.building);
  EXPECT_EQ(info.timestamp, 0);
}

TEST(JenkinsBuildInfoTest, BuildInfoBuildingTrue) {
  build_info info;
  info.number = 1;
  info.status = "RUNNING";
  info.building = true;

  EXPECT_TRUE(info.building);
  EXPECT_EQ(info.status, "RUNNING");
}

TEST(JenkinsBuildInfoTest, BuildInfoListEmptyByDefault) {
  build_info_list builds;
  EXPECT_TRUE(builds.empty());
  EXPECT_EQ(builds.size(), 0u);
}

TEST(JenkinsBuildInfoTest, BuildInfoListWithMultipleBuilds) {
  build_info_list builds;

  build_info b1;
  b1.number = 1;
  b1.status = "SUCCESS";
  b1.building = false;
  builds.push_back(b1);

  build_info b2;
  b2.number = 2;
  b2.status = "FAILURE";
  b2.building = false;
  builds.push_back(b2);

  build_info b3;
  b3.number = 3;
  b3.status = "RUNNING";
  b3.building = true;
  builds.push_back(b3);

  EXPECT_EQ(builds.size(), 3u);
  EXPECT_EQ(builds[0].number, 1);
  EXPECT_EQ(builds[1].number, 2);
  EXPECT_EQ(builds[2].number, 3);
  EXPECT_TRUE(builds[2].building);
}

// ============================================================================
// Edge cases — краевые значения
// ============================================================================

TEST(JenkinsClientConfigTest, ConfigWithVeryLongValues) {
  client::config cfg;
  cfg.base_url = std::string(255, 'a') + ".example.com";
  cfg.username = std::string(255, 'x');
  cfg.api_token = std::string(512, 'y');

  EXPECT_NO_THROW(client c(cfg));
}

TEST(JenkinsClientConfigTest, ConfigWithSpecialCharactersInUsername) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin+user@test";
  cfg.api_token = "token-with-dashes";

  EXPECT_NO_THROW(client c(cfg));
}

TEST(JenkinsClientConfigTest, ConfigWithZeroTimeout) {
  client::config cfg;
  cfg.base_url = "https://jenkins.example.com";
  cfg.username = "admin";
  cfg.api_token = "token";
  cfg.timeout_seconds = 0;

  EXPECT_NO_THROW(client c(cfg));
}

TEST(JenkinsClientBuildInfoTest, BuildInfoWithMaxNumber) {
  build_info info;
  info.number = std::numeric_limits<int>::max();
  info.status = "SUCCESS";
  info.building = false;
  info.timestamp = std::numeric_limits<uint64_t>::max();

  EXPECT_EQ(info.number, std::numeric_limits<int>::max());
  EXPECT_EQ(info.timestamp, std::numeric_limits<uint64_t>::max());
}
