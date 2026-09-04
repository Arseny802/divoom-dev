#include "ics/cache.h"

namespace divoomdev::ics {

ics_cache::ics_cache(std::chrono::milliseconds ttl): ttl_(ttl) { }

void ics_cache::set_ttl(std::chrono::milliseconds ttl) {
  ttl_ = ttl;
}

std::chrono::milliseconds ics_cache::ttl() const noexcept {
  return ttl_;
}

bool ics_cache::is_fresh(const entry& e) const {
  auto age = clock::now() - e.fetched_at;
  return age < ttl_;
}

bool ics_cache::get(const std::string& url, std::string& out) const {
  auto it = store_.find(url);
  if (it == store_.end() || !is_fresh(it->second))
    return false;
  out = it->second.content;
  return true;
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

}  // namespace divoomdev::ics
