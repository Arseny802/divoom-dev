#pragma once
#include <chrono>
#include <map>
#include <string>

namespace divoomdev::ics {

/// Temporary in-memory cache of raw calendar contents with a time-to-live (TTL).
///
/// Features:
/// - In-memory caching with configurable TTL.
/// - Stale data fallback: returns expired data when network fails.
/// - Optional persistent storage to disk for survival across restarts.
/// - Thread-safe for reads once populated by a single writer.
class ics_cache {
 public:
  using clock = std::chrono::system_clock;
  using time_point = clock::time_point;

  /// Configuration for the cache.
  struct options {
    /// TTL for fresh data (default: 5 minutes).
    std::chrono::milliseconds ttl{std::chrono::minutes(5)};
    /// Extra time to serve stale data after TTL expires (default: 60 minutes).
    std::chrono::milliseconds stale_ttl{std::chrono::hours(1)};
    /// Path to persist cache on disk (empty = memory-only).
    std::string persist_path;
  };

  /// Creates a cache with default options.
  ics_cache();

  /// Creates a cache with custom options.
  explicit ics_cache(options opts);

  /// Creates a cache with legacy TTL-only parameter (for backwards compatibility).
  explicit ics_cache(std::chrono::milliseconds ttl);

  void set_ttl(std::chrono::milliseconds ttl);
  std::chrono::milliseconds ttl() const noexcept;

  void set_stale_ttl(std::chrono::milliseconds stale_ttl);
  std::chrono::milliseconds stale_ttl() const noexcept;

  /// Returns true and fills `out` with a fresh (non-expired) entry for the URL.
  bool get(const std::string& url, std::string& out) const;

  /// Returns true and fills `out` with a stale (expired but still valid) entry.
  /// This is useful for fallback when network fails.
  bool get_stale(const std::string& url, std::string& out) const;

  /// Returns whether the entry exists and is still valid (fresh or stale).
  bool has(const std::string& url) const;

  /// Returns true if the entry is still within stale TTL.
  bool is_stale_fresh(const std::string& url) const;

  /// Stores content for the URL, replacing any previous entry and resetting age.
  void put(const std::string& url, std::string content);

  void remove(const std::string& url);
  void clear();

  /// Number of stored URLs (including expired-but-not-yet-pruned entries).
  std::size_t size() const noexcept;

  /// Persists the cache to disk. Returns true on success.
  bool save() const;

  /// Loads the cache from disk. Returns true on success.
  bool load();

 private:
  struct entry {
    std::string content;
    time_point fetched_at;
  };

  bool is_fresh(const entry& e) const;
  bool is_stale(const entry& e) const;
  time_point stale_deadline() const;

  std::map<std::string, entry> store_;
  std::chrono::milliseconds ttl_;
  std::chrono::milliseconds stale_ttl_;
  std::string persist_path_;
};

}  // namespace divoomdev::ics
