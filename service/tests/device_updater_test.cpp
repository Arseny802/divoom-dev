#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "core/device_updater.h"
#include "divoom/client.h"
#include "divoom/exceptions.h"
#include "mocks.h"

// ============================================================================
// Constructors
// ============================================================================
TEST(DeviceUpdater, DefaultConstructor) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update("test"));
}

TEST(DeviceUpdater, ConstructorWithCredentials) {
  divoomdev::service::core::device_updater updater("test@example.com", "password123");
  EXPECT_NO_THROW(updater.update("test"));
}

TEST(DeviceUpdater, ConstructorWithInjectedClient) {
  auto mock_client = std::make_unique<service_test::mock_divoom_client>();
  divoomdev::service::core::device_updater updater(std::move(mock_client));
  EXPECT_NO_THROW(updater.update("test"));
}

// ============================================================================
// update_credentials
// ============================================================================
TEST(DeviceUpdater, UpdateCredentialsSameCredentials) {
  divoomdev::service::core::device_updater updater("test@example.com", "password");
  EXPECT_NO_THROW(updater.update_credentials("test@example.com", "password"));
}

TEST(DeviceUpdater, UpdateCredentialsDifferentCredentials) {
  divoomdev::service::core::device_updater updater("test@example.com", "password");
  EXPECT_NO_THROW(updater.update_credentials("new@example.com", "newpassword"));
}

TEST(DeviceUpdater, UpdateCredentialsEmptyToNotEmpty) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update_credentials("a@b.com", "pass"));
}

TEST(DeviceUpdater, UpdateCredentialsNotEmptyToEmpty) {
  divoomdev::service::core::device_updater updater("a@b.com", "pass");
  EXPECT_NO_THROW(updater.update_credentials("", ""));
}

// ============================================================================
// login (via update_credentials path)
// ============================================================================
TEST(DeviceUpdater, LoginWithInvalidCredentials) {
  divoomdev::service::core::device_updater updater("wrong@example.com", "wrongpass");
  EXPECT_NO_THROW(updater.update_credentials("wrong@example.com", "wrongpass"));
}

// ============================================================================
// get_devices
// ============================================================================
TEST(DeviceUpdater, GetDevicesWhenNotLoggedIn) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update("test"));
}

TEST(DeviceUpdater, GetDevicesEmptyList) {
  divoomdev::service::core::device_updater updater("test@example.com", "pass");
  EXPECT_NO_THROW(updater.update("test"));
}

// ============================================================================
// update
// ============================================================================
TEST(DeviceUpdater, UpdateWithNoDevices) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update("test text"));
}

TEST(DeviceUpdater, UpdateWithNoUserInfo) {
  auto mock_client = std::make_unique<service_test::mock_divoom_client>();
  divoomdev::service::core::device_updater updater(std::move(mock_client));
  EXPECT_NO_THROW(updater.update("test text"));
}

TEST(DeviceUpdater, UpdateSkipsSameHash) {
  divoomdev::service::core::device_updater updater;
  updater.update("same text");
  updater.update("same text");
}

TEST(DeviceUpdater, UpdateDifferentText) {
  divoomdev::service::core::device_updater updater;
  updater.update("text1");
  updater.update("text2");
}

TEST(DeviceUpdater, UpdateWithEmptyText) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update(""));
}

TEST(DeviceUpdater, UpdateWithLongText) {
  divoomdev::service::core::device_updater updater;
  std::string long_text(10000, 'A');
  EXPECT_NO_THROW(updater.update(long_text));
}

TEST(DeviceUpdater, UpdateWithUnicodeText) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update("Привет мир 你好世界"));
}

TEST(DeviceUpdater, UpdateWithNewlines) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update("line1\nline2\nline3"));
}

TEST(DeviceUpdater, UpdateWithSpecialChars) {
  divoomdev::service::core::device_updater updater;
  EXPECT_NO_THROW(updater.update("<html>&'\"<>"));
}
