#pragma once
#include <chrono>
#include <map>
#include <string>

namespace divoomdev::ics {

/// Temporary in-memory cache of raw calendar contents with a time-to-live (TTL).
///
/// Used to avoid re-fetching the same URL within a short window. It is safe to
/// use across threads for reads once populated by a single writer.
class ics_cache {
 public:
  using clock = std::chrono::system_clock;
  using time_point = clock::time_point;

  /// Creates a cache; entries older than `ttl` are considered stale.
  explicit ics_cache(std::chrono::milliseconds ttl = std::chrono::minutes(5));

  void set_ttl(std::chrono::milliseconds ttl);
  std::chrono::milliseconds ttl() const noexcept;

  /// Returns true and fills `out` with a fresh (non-expired) entry for the URL.
  bool get(const std::string& url, std::string& out) const;

  /// Stores content for the URL, replacing any previous entry and resetting age.
  void put(const std::string& url, std::string content);

  void remove(const std::string& url);
  void clear();

  /// Number of stored URLs (including expired-but-not-yet-pruned entries).
  std::size_t size() const noexcept;

 private:
  struct entry {
    std::string content;
    time_point fetched_at;
  };

  bool is_fresh(const entry& e) const;

  std::map<std::string, entry> store_;
  std::chrono::milliseconds ttl_;
};

}  // namespace divoomdev::ics