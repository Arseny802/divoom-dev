#include "jenkins/client.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

namespace divoomdev::jenkins {
// Безопасная таблица символов Base64
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

// Исправленная функция кодирования
std::string EncodeBase64(const unsigned char* bytes_to_encode, size_t in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    // Корректное добавление паддинга '=' (в прошлой версии его не хватало или он был неверным)
    while ((i++ < 3))
      ret += '=';
  }

  return ret;
}

// Вспомогательная обертка для строк
std::string EncodeBase64(const std::string& input) {
  return EncodeBase64(reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
}

cpr::Header PrepareAuthHeaders(const std::string& username, const std::string& token) {
  std::string creds = username + ":" + token;
  std::string encoded_creds = EncodeBase64(creds);

  cpr::Header headers{{"Accept", "application/json"}, {"Authorization", "Basic " + encoded_creds}};
  return headers;
}

client::client(const config& config): config_(config) { }

build_info_list client::ParseBuildsJson(const std::string& json_string, const std::string& user_id) {
  build_info_list builds;
  try {
    auto json = nlohmann::json::parse(json_string);
    bool filter_by_user = !user_id.empty();

    for (const auto& build_json: json["builds"]) {
      // Пропускаем элементы очереди (они имеют поле "why")
      if (build_json.contains("why") && build_json["why"].is_string())
        continue;

      build_info info;
      info.number = build_json.value("number", 0);
      info.building = build_json.value("building", false);

      // Jenkins может возвращать null в поле result для идущих сборок
      if (!build_json.value("result", nlohmann::json()).is_null()) {
        info.status = build_json.value("result", "UNKNOWN");
      } else {
        info.status = "IN_PROGRESS";
      }
      info.result = info.status;
      info.timestamp = build_json.value("timestamp", 0ULL);

      bool user_matches = true;
      if (filter_by_user) {
        user_matches = false;
        if (build_json.contains("actions")) {
          for (const auto& action: build_json["actions"]) {
            if (action.contains("causes")) {
              for (const auto& cause: action["causes"]) {
                // Проверяем как userId, так и username (для старых версий Jenkins)
                if ((cause.contains("userId") && cause["userId"] == user_id) ||
                    (cause.contains("username") && cause["username"] == user_id)) {
                  user_matches = true;
                  break;
                }
              }
            }
            if (user_matches)
              break;
          }
        }
      }

      if (user_matches) {
        builds.push_back(info);
      }
    }
  } catch (const nlohmann::json::exception& e) {
    log()->error("JSON parsing error: {}", e.what());
  }
  return builds;
}

build_info_list client::GetUserBuilds(const std::string& job_name, const std::string& user_id, int count) {
  std::string url_path = "/job/" + job_name + "/api/json";

  cpr::Parameters params{{"tree", "builds[number,timestamp,result,building,actions[causes[userId,username]]]"},
                         {"depth", "1"}};
  if (count > 0 && count < 1000) {
    params.Add({"max", std::to_string(count)});
  }

  auto response = cpr::Get(cpr::Url{config_.base_url + url_path},
                           PrepareAuthHeaders(config_.username, config_.api_token),
                           params,
                           cpr::Timeout{config_.timeout_seconds * 1000});
  if (response.text.find("<!DOCTYPE") != std::string::npos || response.text.find("<html") != std::string::npos) {
    log()->error("--- SERVER RETURNED HTML INSTEAD OF JSON ---");
    // Выведем начало текста, чтобы понять причину (Login page? 404?)
    log()->error(response.text.substr(0, 1000));
    return {};
  }

  if (response.error.code != cpr::ErrorCode::OK) {
    log()->error("HTTP request failed: ", response.error.message);
    log()->error("Status code: ", response.status_code);
    return {};
  }

  // Дополнительная проверка: Jenkins при ошибке авторизации часто отдает JSON с ошибкой внутри тела ответа 200 OK
  try {
    auto check_auth = nlohmann::json::parse(response.text);
    if (check_auth.contains("status") && check_auth["status"] == 401) {
      log()->error("Authentication failed: Invalid credentials or insufficient permissions.");
      return {};
    }
    if (check_auth.contains("error")) {
      log()->error("Jenkins API Error: {}", check_auth["error"].get<std::string>());
      return {};
    }
  } catch (...) {
    // Если это не JSON, просто идем дальше к парсингу билдов
  }

  return ParseBuildsJson(response.text, user_id);
}
}  // namespace divoomdev::jenkins
