#include "gtest/gtest.h"

#include <thread>

#include "ics/cache.h"

using namespace divoomdev::ics;

TEST(IcsCache, PutAndGet) {
  ics_cache cache(std::chrono::seconds(60));
  EXPECT_EQ(cache.size(), 0u);

  cache.put("url-1", "content-1");
  cache.put("url-2", "content-2");
  ASSERT_EQ(cache.size(), 2u);

  std::string out;
  EXPECT_TRUE(cache.get("url-1", out));
  EXPECT_EQ(out, "content-1");
  EXPECT_TRUE(cache.get("url-2", out));
  EXPECT_EQ(out, "content-2");
}

TEST(IcsCache, PutReplacesEntry) {
  ics_cache cache(std::chrono::seconds(60));
  cache.put("u", "v1");
  cache.put("u", "v2");

  std::string out;
  ASSERT_TRUE(cache.get("u", out));
  EXPECT_EQ(out, "v2");
  EXPECT_EQ(cache.size(), 1u);
}

TEST(IcsCache, MissingUrlReturnsFalse) {
  ics_cache cache(std::chrono::seconds(60));
  std::string out;
  EXPECT_FALSE(cache.get("missing", out));
}

TEST(IcsCache, ExpiredEntryIsNotReturned) {
  ics_cache cache(std::chrono::milliseconds(30));
  cache.put("u", "content");

  std::this_thread::sleep_for(std::chrono::milliseconds(60));
  std::string out;
  EXPECT_FALSE(cache.get("u", out));
}

TEST(IcsCache, RemoveAndClear) {
  ics_cache cache(std::chrono::seconds(60));
  cache.put("a", "1");
  cache.put("b", "2");

  cache.remove("a");
  std::string out;
  EXPECT_FALSE(cache.get("a", out));

  cache.clear();
  EXPECT_EQ(cache.size(), 0u);
}

TEST(IcsCache, SetTtlChangesFreshness) {
  ics_cache cache(std::chrono::milliseconds(10));
  cache.put("u", "x");

  cache.set_ttl(std::chrono::minutes(5));
  std::string out;
  EXPECT_TRUE(cache.get("u", out));
  EXPECT_EQ(cache.ttl(), std::chrono::minutes(5));
}

TEST(IcsCache, StaleDataFallback) {
  // Fresh TTL is short, stale TTL is long
  ics_cache cache(std::chrono::milliseconds(30));
  cache.set_stale_ttl(std::chrono::seconds(10));

  cache.put("u", "old-content");

  // Initially fresh
  std::string out;
  EXPECT_TRUE(cache.get("u", out));
  EXPECT_EQ(out, "old-content");

  // After fresh TTL expires, get() fails but get_stale() works
  std::this_thread::sleep_for(std::chrono::milliseconds(60));
  EXPECT_FALSE(cache.get("u", out));
  EXPECT_TRUE(cache.get_stale("u", out));
  EXPECT_EQ(out, "old-content");
}

TEST(IcsCache, StaleDataEventuallyExpires) {
  ics_cache cache(std::chrono::milliseconds(20));
  cache.set_stale_ttl(std::chrono::milliseconds(50));

  cache.put("u", "content");

  // Wait for both fresh and stale TTL to expire
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::string out;
  EXPECT_FALSE(cache.get("u", out));
  EXPECT_FALSE(cache.get_stale("u", out));
  EXPECT_FALSE(cache.has("u"));
}

TEST(IcsCache, HasAndIsStaleFresh) {
  ics_cache cache(std::chrono::milliseconds(30));
  cache.set_stale_ttl(std::chrono::seconds(10));

  cache.put("u", "content");

  // Initially fresh
  EXPECT_TRUE(cache.has("u"));
  EXPECT_FALSE(cache.is_stale_fresh("u"));

  // After fresh TTL expires
  std::this_thread::sleep_for(std::chrono::milliseconds(60));
  EXPECT_TRUE(cache.has("u"));
  EXPECT_TRUE(cache.is_stale_fresh("u"));
}