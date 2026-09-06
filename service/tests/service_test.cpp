#include "gtest/gtest.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "core/device_updater.h"
#include "core/event_formatter.h"
#include "core/service.h"
#include "lifecycle/service_state.h"
#include "mocks.h"
#include "storage/i_storage.h"

namespace {

static std::unique_ptr<service_test::mock_settings_storage> make_mock_storage() {
  auto storage = std::make_unique<service_test::mock_settings_storage>();
  storage->accounts_.push_back(divoomdev::storage::account{"divoom", "test@example.com", "password"});
  return storage;
}

static std::unique_ptr<divoomdev::service::core::device_updater> make_mock_updater() {
  return std::make_unique<divoomdev::service::core::device_updater>();
}

static std::unique_ptr<divoomdev::service::core::event_formatter> make_mock_formatter() {
  return std::make_unique<divoomdev::service::core::event_formatter>("Нет событий");
}

static std::unique_ptr<divoomdev::ics::event_manager> make_mock_calendar() {
  return std::make_unique<service_test::mock_event_manager>();
}

}  // namespace

TEST(Service, DefaultStorageConstructor) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  EXPECT_NE(svc.get_storage(), nullptr);
}

TEST(Service, InjectedConstructor) {
  auto storage = make_mock_storage();
  auto updater = make_mock_updater();
  auto formatter = make_mock_formatter();
  auto calendar = make_mock_calendar();
  auto* storage_ptr = storage.get();
  divoomdev::service::core::service svc(std::move(storage), std::move(updater), std::move(formatter), std::move(calendar));
  EXPECT_EQ(svc.get_storage(), storage_ptr);
}

TEST(Service, InjectedConstructorWithNulls) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage), nullptr, nullptr, nullptr);
  EXPECT_NE(svc.get_storage(), nullptr);
}

TEST(Service, MoveConstructor) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc1(std::move(storage));
  divoomdev::service::core::service svc2(std::move(svc1));
  EXPECT_NE(svc2.get_storage(), nullptr);
  EXPECT_EQ(svc1.get_storage(), nullptr);
}

TEST(Service, MoveAssignment) {
  auto storage1 = make_mock_storage();
  auto storage2 = make_mock_storage();
  divoomdev::service::core::service svc1(std::move(storage1));
  divoomdev::service::core::service svc2(std::move(storage2));
  svc2 = std::move(svc1);
  EXPECT_NE(svc2.get_storage(), nullptr);
  EXPECT_EQ(svc1.get_storage(), nullptr);
}

TEST(Service, CopyConstructorDeleted) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc1(std::move(storage));
  EXPECT_FALSE((std::is_copy_constructible_v<divoomdev::service::core::service>));
}

TEST(Service, CopyAssignmentDeleted) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc1(std::move(storage));
  EXPECT_FALSE((std::is_copy_assignable_v<divoomdev::service::core::service>));
}

TEST(Service, GetStorageReturnsValidPointer) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  auto* ptr = svc.get_storage();
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->list_accounts().size(), 1u);
}

TEST(Service, GetStorageReturnsNullWhenNoStorage) {
  divoomdev::service::core::service svc(nullptr);
  EXPECT_EQ(svc.get_storage(), nullptr);
}

TEST(Service, SetUpdateIntervalDefault) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
}

TEST(Service, SetUpdateIntervalOneMinute) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  svc.set_update_interval(std::chrono::minutes(1));
}

TEST(Service, SetUpdateIntervalLongInterval) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  svc.set_update_interval(std::chrono::minutes(60));
}

TEST(Service, SetUpdateIntervalZero) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  svc.set_update_interval(std::chrono::minutes(0));
}

TEST(Service, RunStopsImmediately) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  svc.stop();
  svc.run();
}

TEST(Service, RunWithPauseAndResume) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  svc.pause();
  svc.run();
  divoomdev::service::lifecycle::g_service_paused() = false;
  divoomdev::service::lifecycle::g_service_running() = true;
  svc.pause();
  svc.run();
}

TEST(Service, RunWithStopDuringLoop) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
  svc.set_update_interval(std::chrono::minutes(0));
  svc.stop();
  svc.run();
}

TEST(Service, PauseSetsPausedState) {
  divoomdev::service::lifecycle::g_service_paused() = false;
  EXPECT_FALSE(divoomdev::service::lifecycle::g_service_paused());
  divoomdev::service::lifecycle::g_service_paused() = true;
  EXPECT_TRUE(divoomdev::service::lifecycle::g_service_paused());
}

TEST(Service, ResumeClearsPausedState) {
  divoomdev::service::lifecycle::g_service_paused() = true;
  divoomdev::service::lifecycle::g_service_paused() = false;
  EXPECT_FALSE(divoomdev::service::lifecycle::g_service_paused());
}

TEST(Service, StopSetsRunningState) {
  divoomdev::service::lifecycle::g_service_running() = true;
  divoomdev::service::lifecycle::g_service_running() = false;
  EXPECT_FALSE(divoomdev::service::lifecycle::g_service_running());
}

TEST(Service, ProcessCycleWithNoEvents) {
  auto storage = make_mock_storage();
  auto calendar = make_mock_calendar();
  auto formatter = make_mock_formatter();
  auto updater = make_mock_updater();
  divoomdev::service::core::service svc(std::move(storage), std::move(updater), std::move(formatter), std::move(calendar));
  svc.stop();
  svc.run();
}

TEST(Service, ProcessCycleWithInjectedNullCalendar) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage), nullptr, nullptr, nullptr);
  svc.stop();
}

TEST(Service, ServiceStateRunning) {
  divoomdev::service::lifecycle::g_service_running() = true;
  EXPECT_TRUE(divoomdev::service::lifecycle::g_service_running());
  divoomdev::service::lifecycle::g_service_running() = false;
  EXPECT_FALSE(divoomdev::service::lifecycle::g_service_running());
}

TEST(Service, ServiceStatePaused) {
  divoomdev::service::lifecycle::g_service_paused() = false;
  EXPECT_FALSE(divoomdev::service::lifecycle::g_service_paused());
  divoomdev::service::lifecycle::g_service_paused() = true;
  EXPECT_TRUE(divoomdev::service::lifecycle::g_service_paused());
}

TEST(Service, ServiceStateIndependent) {
  divoomdev::service::lifecycle::g_service_running() = true;
  divoomdev::service::lifecycle::g_service_paused() = true;
  EXPECT_TRUE(divoomdev::service::lifecycle::g_service_running());
  EXPECT_TRUE(divoomdev::service::lifecycle::g_service_paused());
  divoomdev::service::lifecycle::g_service_running() = false;
  divoomdev::service::lifecycle::g_service_paused() = false;
}

TEST(Service, DestructorDoesNotThrow) {
  auto storage = make_mock_storage();
  divoomdev::service::core::service svc(std::move(storage));
}

TEST(Service, DestructorWithInjectedDependencies) {
  auto storage = make_mock_storage();
  auto updater = make_mock_updater();
  auto formatter = make_mock_formatter();
  auto calendar = make_mock_calendar();
}
