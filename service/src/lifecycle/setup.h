#pragma once
#include <chrono>
#include <string_view>

namespace divoomdev::service::lifecycle {

struct setup {
  static constexpr std::chrono::minutes UPDATE_INTERVAL = std::chrono::minutes(15);

  static constexpr char SERVICE_NAME[] = "DivoomDev";
  static constexpr char SERVICE_DISPLAY_NAME[] = "Divoom Developer Service";
  static constexpr char SERVICE_DESCRIPTION[] = "Divoom device clock manager service";

#if defined(_WIN32) || defined(_WIN64)
  static constexpr wchar_t SERVICE_NAME_W[] = L"DivoomDev";
  static constexpr wchar_t SERVICE_DISPLAY_NAME_W[] = L"Divoom Developer Service";
  static constexpr wchar_t SERVICE_DESCRIPTION_W[] = L"Divoom device clock manager service";

  static constexpr std::string_view ORG_NAME = "arseny802";
  static constexpr std::string_view DATA_DIR = "C:/ProgramData/arseny802/divoomdev";
  static constexpr std::string_view LOG_DIR = "C:/ProgramData/arseny802/divoomdev/logs";
#else
  static constexpr std::string_view ORG_NAME = "arseny802";
  static constexpr std::string_view DATA_DIR = "/var/lib/arseny802/divoomdev";
  static constexpr std::string_view LOG_DIR = "/var/log/arseny802/divoomdev";
#endif

  static constexpr std::string_view DATABASE_FILE = "config.db";
};

}  // namespace divoomdev::service::lifecycle
