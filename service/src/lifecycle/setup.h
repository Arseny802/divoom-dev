#pragma once
#include <chrono>

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
#endif
};

}  // namespace divoomdev::service::lifecycle
