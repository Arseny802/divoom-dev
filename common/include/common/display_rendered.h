#pragma once
#include "display.h"
#include "display_type.h"
#include <magic_enum/magic_enum.hpp>

namespace divoomdev::common {

struct display_rendered : display {
  display_type type = display_type::Text;
  /// It will be display element
  std::string Type = magic_enum::enum_name(display_type::Text).data();
  /// Display the starting position x coordinate of the display area
  int StartX = 0;
  /// Display the starting position y coordinate of the display area
  int StartY = 0;
  /// Display area width
  int Width = 0;
  /// Display area height
  int Height = 0;
  /// 0；left； 1：right; 2:middle
  int Align = 0;
  std::string FontColor = "#FFFFFF";
  std::string BgColor = "#000000";

  // Text display type

  /// Display font size, only text is valid
  int FontSize = 0;
  /// Display font ID, only text is valid
  int FontID = 0;

  // NetData display type

  /// It is url address, Network request address or image download address.
  std::string Url{};
  /// This is the data parsing rule for network requests, which can be referred to
  /// “https://docin.divoom-gz.com/web/#/5/145”.
  std::string RuleInfo{};
  /// Network request interval time, in seconds, greater than 10
  int RequestTime = 30;
};
using display_rendered_list = std::vector<display_rendered>;

#ifdef NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(display_rendered,
                                   ID,
                                   TextMessage,
                                   Type,
                                   StartX,
                                   StartY,
                                   Width,
                                   Height,
                                   FontSize,
                                   FontID,
                                   FontColor,
                                   BgColor,
                                   Url,
                                   RuleInfo,
                                   RequestTime);
#endif

}  // namespace divoomdev::common
