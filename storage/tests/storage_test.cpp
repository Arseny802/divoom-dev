#include "gtest/gtest.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "storage/backend.h"
#include "storage/i_storage.h"
#include "storage/models.h"
#include "storage/storage.hpp"

namespace {

using divoomdev::storage::account;
using divoomdev::storage::backend_type;
using divoomdev::storage::calendar_source;
using divoomdev::storage::i_settings_storage;
using divoomdev::storage::open_storage;

struct backend_case {
  backend_type type;
  std::string file;
};

class StorageTest : public ::testing::TestWithParam<backend_case> {
 protected:
  void SetUp() override {
    static int counter = 0;
    dir_ = std::filesystem::temp_directory_path() / ("divoomdev_storage_test_" + std::to_string(counter++));
    std::filesystem::create_directories(dir_);
    path_ = (dir_ / GetParam().file).string();
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(dir_, ec);
  }

  std::unique_ptr<i_settings_storage> open() const { return open_storage(GetParam().type, path_); }

  std::filesystem::path dir_;
  std::string path_;
};

INSTANTIATE_TEST_SUITE_P(Backends,
                         StorageTest,
                         ::testing::Values(backend_case{backend_type::sqlite, "test.db"},
                                           backend_case{backend_type::json_file, "config.json"}),
                         [](const ::testing::TestParamInfo<backend_case>& info) {
                           return std::string(divoomdev::storage::to_string(info.param.type));
                         });

TEST_P(StorageTest, OpensEmptyStore) {
  auto store = open();
  ASSERT_NE(store, nullptr);
  EXPECT_TRUE(store->list_accounts().empty());
  EXPECT_TRUE(store->list_calendar_sources().empty());
}

TEST_P(StorageTest, UpsertAndGetAccount) {
  auto store = open();
  ASSERT_NE(store, nullptr);

  store->upsert_account(account{"divoom", "user@example.com", "pass"});
  auto acc = store->get_account("divoom");
  ASSERT_TRUE(acc.has_value());
  EXPECT_EQ(acc->service, "divoom");
  EXPECT_EQ(acc->login, "user@example.com");
  EXPECT_EQ(acc->password, "pass");

  EXPECT_FALSE(store->get_account("missing").has_value());
}

TEST_P(StorageTest, UpsertAccountOverridesExisting) {
  auto store = open();
  ASSERT_NE(store, nullptr);

  store->upsert_account(account{"divoom", "a@b.c", "one"});
  store->upsert_account(account{"divoom", "a@b.c", "two"});

  auto accounts = store->list_accounts();
  ASSERT_EQ(accounts.size(), 1u);
  EXPECT_EQ(accounts[0].password, "two");
}

TEST_P(StorageTest, ListAndDeleteAccounts) {
  auto store = open();
  ASSERT_NE(store, nullptr);

  store->upsert_account(account{"s1", "l1", "p1"});
  store->upsert_account(account{"s2", "l2", "p2"});
  EXPECT_EQ(store->list_accounts().size(), 2u);

  store->delete_account("s1");
  EXPECT_EQ(store->list_accounts().size(), 1u);
  EXPECT_FALSE(store->get_account("s1").has_value());
  EXPECT_TRUE(store->get_account("s2").has_value());

  // Удаление несуществующей записи не является ошибкой.
  store->delete_account("s1");
  EXPECT_EQ(store->list_accounts().size(), 1u);
}

TEST_P(StorageTest, UpsertCalendarSourcesSortedByPriority) {
  auto store = open();
  ASSERT_NE(store, nullptr);

  store->upsert_calendar_source(calendar_source{"https://c/low.ics", 1, true});
  store->upsert_calendar_source(calendar_source{"https://c/high.ics", 10, false});
  store->upsert_calendar_source(calendar_source{"https://c/mid.ics", 5, true});

  auto sources = store->list_calendar_sources();
  ASSERT_EQ(sources.size(), 3u);
  EXPECT_EQ(sources[0].url, "https://c/high.ics");
  EXPECT_EQ(sources[1].url, "https://c/mid.ics");
  EXPECT_EQ(sources[2].url, "https://c/low.ics");
  EXPECT_FALSE(sources[0].enabled);
}

TEST_P(StorageTest, UpdateAndRemoveCalendarSource) {
  auto store = open();
  ASSERT_NE(store, nullptr);

  store->upsert_calendar_source(calendar_source{"https://c/a.ics", 1, true});
  store->upsert_calendar_source(calendar_source{"https://c/a.ics", 99, false});

  auto sources = store->list_calendar_sources();
  ASSERT_EQ(sources.size(), 1u);
  EXPECT_EQ(sources[0].priority, 99);
  EXPECT_FALSE(sources[0].enabled);

  store->remove_calendar_source("https://c/a.ics");
  EXPECT_TRUE(store->list_calendar_sources().empty());

  store->remove_calendar_source("https://c/a.ics");  // no error
}

TEST_P(StorageTest, PersistsAcrossReopen) {
  {
    auto store = open();
    ASSERT_NE(store, nullptr);
    store->upsert_account(account{"divoom", "persist@example.com", "pw"});
    store->upsert_calendar_source(calendar_source{"https://c/persist.ics", 7, true});
  }  // закрываем хранилище

  auto store = open();
  ASSERT_NE(store, nullptr);
  auto acc = store->get_account("divoom");
  ASSERT_TRUE(acc.has_value());
  EXPECT_EQ(acc->password, "pw");

  auto sources = store->list_calendar_sources();
  ASSERT_EQ(sources.size(), 1u);
  EXPECT_EQ(sources[0].priority, 7);
}

TEST_P(StorageTest, ClearRemovesAllData) {
  auto store = open();
  ASSERT_NE(store, nullptr);

  store->upsert_account(account{"s1", "l", "p"});
  store->upsert_calendar_source(calendar_source{"https://c/x.ics", 1, true});

  store->clear();

  EXPECT_TRUE(store->list_accounts().empty());
  EXPECT_TRUE(store->list_calendar_sources().empty());
}

}  // namespace