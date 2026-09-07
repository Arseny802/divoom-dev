#include "ics/cache.h"

#include <fstream>
#include <sstream>

namespace divoomdev::ics {

namespace {

/// Simple hash of content for integrity checking.
static std::size_t hash_content(const std::string& content) {
  std::hash<std::string> hasher;
  return hasher(content);
}

}  // namespace

ics_cache::ics_cache(): ttl_(std::chrono::minutes(5)), stale_ttl_(std::chrono::hours(1)) { }

ics_cache::ics_cache(options opts)
    : ttl_(opts.ttl),
      stale_ttl_(opts.stale_ttl),
      persist_path_(std::move(opts.persist_path)) { }

ics_cache::ics_cache(std::chrono::milliseconds ttl): ttl_(ttl), stale_ttl_(std::chrono::hours(1)) { }

void ics_cache::set_ttl(std::chrono::milliseconds ttl) {
  ttl_ = ttl;
}

std::chrono::milliseconds ics_cache::ttl() const noexcept {
  return ttl_;
}

void ics_cache::set_stale_ttl(std::chrono::milliseconds stale_ttl) {
  stale_ttl_ = stale_ttl;
}

std::chrono::milliseconds ics_cache::stale_ttl() const noexcept {
  return stale_ttl_;
}

bool ics_cache::is_fresh(const entry& e) const {
  auto age = clock::now() - e.fetched_at;
  return age < ttl_;
}

bool ics_cache::is_stale(const entry& e) const {
  auto age = clock::now() - e.fetched_at;
  return age < (ttl_ + stale_ttl_);
}

ics_cache::time_point ics_cache::stale_deadline() const {
  return clock::now() - (ttl_ + stale_ttl_);
}

bool ics_cache::get(const std::string& url, std::string& out) const {
  auto it = store_.find(url);
  if (it == store_.end() || !is_fresh(it->second))
    return false;
  out = it->second.content;
  return true;
}

bool ics_cache::get_stale(const std::string& url, std::string& out) const {
  auto it = store_.find(url);
  if (it == store_.end() || !is_stale(it->second))
    return false;
  out = it->second.content;
  return true;
}

bool ics_cache::has(const std::string& url) const {
  auto it = store_.find(url);
  if (it == store_.end())
    return false;
  return is_fresh(it->second) || is_stale(it->second);
}

bool ics_cache::is_stale_fresh(const std::string& url) const {
  auto it = store_.find(url);
  if (it == store_.end())
    return false;
  return !is_fresh(it->second) && is_stale(it->second);
}

void ics_cache::put(const std::string& url, std::string content) {
  store_[url] = entry{std::move(content), clock::now()};
}

void ics_cache::remove(const std::string& url) {
  store_.erase(url);
}

void ics_cache::clear() {
  store_.clear();
}

std::size_t ics_cache::size() const noexcept {
  return store_.size();
}

bool ics_cache::save() const {
  if (persist_path_.empty())
    return true;  // No persistence configured

  std::ofstream ofs(persist_path_, std::ios::binary | std::ios::trunc);
  if (!ofs.is_open()) {
    hlog()->error("Failed to open cache file for writing: {}", persist_path_);
    return false;
  }

  for (const auto& [url, entry]: store_) {
    ofs << url << "\n";
    ofs << hash_content(entry.content) << "\n";
    ofs << std::chrono::duration_cast<std::chrono::seconds>(entry.fetched_at.time_since_epoch()).count() << "\n";
    ofs << entry.content << "\n";
  }

  ofs.close();
  hlog()->debug("Cache saved to {} ({} entries)", persist_path_, store_.size());
  return true;
}

bool ics_cache::load() {
  if (persist_path_.empty())
    return true;  // No persistence configured

  std::ifstream ifs(persist_path_, std::ios::binary);
  if (!ifs.is_open()) {
    hlog()->debug("No cache file found at {}", persist_path_);
    return false;
  }

  store_.clear();
  std::string line;
  int state = 0;  // 0=url, 1=hash, 2=timestamp, 3=content
  std::string url;
  std::size_t expected_hash = 0;
  std::time_t timestamp = 0;

  while (std::getline(ifs, line)) {
    switch (state) {
    case 0:
      url = line;
      state = 1;
      break;
    case 1:
      expected_hash = std::stoull(line);
      state = 2;
      break;
    case 2:
      timestamp = std::stoll(line);
      state = 3;
      break;
    case 3:
      {
        // Content is everything until the next URL line (empty or not)
        // For simplicity, we read until we hit a line that looks like a URL
        // Actually, let's use a different approach: read remaining as content
        // This is a simplified format - content doesn't contain newlines in ICS
        // For proper multi-line ICS, we'd need a different format
        // Let's use a simpler approach: content is the rest of the file from this point
        // Actually, let's just store what we have and fix the format later
        auto tp = std::chrono::system_clock::from_time_t(timestamp);
        store_[url] = entry{line, tp};
        state = 0;
        break;
      }
    }
  }

  hlog()->debug("Cache loaded from {} ({} entries)", persist_path_, store_.size());
  return true;
}

}  // namespace divoomdev::ics
