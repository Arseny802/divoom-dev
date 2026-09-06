#pragma once
#include <string>
#include <vector>

#include "build_info.h"

namespace divoomdev::jenkins {

class client {
 public:
  struct config {
    std::string base_url;
    std::string username;
    std::string api_token;
    int timeout_seconds = 30;
  };

  explicit client(const config& config);

  // В качестве user_id лучше передавать логин пользователя из системы безопасности Jenkins
  std::vector<build_info> GetUserBuilds(const std::string& job_name, const std::string& user_id, int count = 50);
  const config& GetConfig() const noexcept { return config_; }

 private:
  config config_;
  std::vector<build_info> ParseBuildsJson(const std::string& json_string, const std::string& user_id);
};

}  // namespace divoomdev::jenkins
