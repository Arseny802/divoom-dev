#pragma once
#include "common/common.hpp"

namespace divoomdev::service {

common::display_rendered_list get_display_list() {
  common::display_rendered_list display_list;
  std::string backgroud_image_addr = "https://f.divoom-gz.com/group1/M00/0C/53/rBAAM2faipuEYhJQAAAAAAsMG7w762.jpg";
  int backgroud_image_local_flag = 0;

  // Item 1: NetData (Score)
  {
    common::display_rendered item;
    item.type = common::display_type::NetData;
    item.ID = 1;
    item.Type = "NetData";
    item.StartX = 491;
    item.StartY = 462;
    item.Width = 272;
    item.Height = 53;
    item.Align = 0;
    item.FontSize = 40;
    item.FontID = 52;
    item.FontColor = "#FFFFFF";
    item.BgColor = "#FF0000";
    item.Url = "https://app.divoom-gz.com/User/GetUserData?SearchDivoomUser=400000012";
    item.RuleInfo = "n:LikeCnt";
    item.RequestTime = 30;
    display_list.emplace_back(item);
  }

  // Item 2: NetData (Level)
  {
    common::display_rendered item;
    item.type = common::display_type::NetData;
    item.Type = "NetData";
    item.ID = 2;
    item.StartX = 483;
    item.StartY = 385;
    item.Width = 280;
    item.Height = 53;
    item.Align = 0;
    item.FontSize = 40;
    item.FontID = 52;
    item.FontColor = "#FFFFFF";
    item.BgColor = "#FF0000";
    item.Url = "https://app.divoom-gz.com/User/GetUserData?SearchDivoomUser=400000012";
    item.RuleInfo = "n:Level";
    item.RequestTime = 30;
    display_list.emplace_back(item);
  }

  // Item 3: NetData (Nickname)
  {
    common::display_rendered item;
    item.type = common::display_type::NetData;
    item.Type = "NetData";
    item.ID = 3;
    item.StartX = 349;
    item.StartY = 252;
    item.Width = 414;
    item.Height = 93;
    item.Align = 0;
    item.FontSize = 64;
    item.FontID = 126;
    item.FontColor = "#FFFFFF";
    item.BgColor = "#FF0000";
    item.Url = "https://app.divoom-gz.com/User/GetUserData?SearchDivoomUser=400000012";
    item.RuleInfo = "s:Nickname";
    item.RequestTime = 30;
    display_list.emplace_back(item);
  }

  // Item 4: Time
  {
    common::display_rendered item;
    item.type = common::display_type::Time;
    item.Type = "Time";
    item.ID = 4;
    item.StartX = 474;
    item.StartY = 1123;
    item.Width = 308;
    item.Height = 125;
    item.Align = 1;
    item.FontSize = 95;
    item.FontID = 52;
    item.FontColor = "#A8B35E";
    item.BgColor = "#FF0000";
    display_list.emplace_back(item);
  }

  // Item 5: Mday
  {
    common::display_rendered item;
    item.type = common::display_type::Mday;
    item.Type = "Mday";
    item.ID = 5;
    item.StartX = 236;
    item.StartY = 1136;
    item.Width = 79;
    item.Height = 50;
    item.Align = 0;
    item.FontSize = 38;
    item.FontID = 52;
    item.FontColor = "#CFAA63";
    item.BgColor = "#FF0000";
    display_list.emplace_back(item);
  }

  // Item 6: MonYear
  {
    common::display_rendered item;
    item.type = common::display_type::MonYear;
    item.Type = "MonYear";
    item.ID = 6;
    item.StartX = 136;
    item.StartY = 1186;
    item.Width = 200;
    item.Height = 50;
    item.Align = 0;
    item.FontSize = 38;
    item.FontID = 52;
    item.FontColor = "#CFAA63";
    item.BgColor = "#FF0000";
    display_list.emplace_back(item);
  }

  // Item 7: Week
  {
    common::display_rendered item;
    item.type = common::display_type::Week;
    item.Type = "Week";
    item.ID = 7;
    item.StartX = 136;
    item.StartY = 1136;
    item.Width = 100;
    item.Height = 50;
    item.Align = 0;
    item.FontSize = 38;
    item.FontID = 52;
    item.FontColor = "#A8B35E";
    item.BgColor = "#FF0000";
    display_list.emplace_back(item);
  }

  // Item 8: NetData (LikeCnt)
  {
    common::display_rendered item;
    item.type = common::display_type::NetData;
    item.Type = "NetData";
    item.ID = 8;
    item.StartX = 51;
    item.StartY = 890;
    item.Width = 699;
    item.Height = 174;
    item.Align = 2;
    item.FontSize = 115;
    item.FontID = 126;
    item.FontColor = "#484D12";
    item.BgColor = "#FF0000";
    item.Url = "https://app.divoom-gz.com/User/GetUserData?SearchDivoomUser=400000012";
    item.RuleInfo = "n:LikeCnt";
    item.RequestTime = 30;
    display_list.emplace_back(item);
  }

  // Item 9: NetData (FansCnt)
  {
    common::display_rendered item;
    item.type = common::display_type::NetData;
    item.Type = "NetData";
    item.ID = 9;
    item.StartX = 51;
    item.StartY = 616;
    item.Width = 699;
    item.Height = 168;
    item.Align = 2;
    item.FontSize = 115;
    item.FontID = 126;
    item.FontColor = "#484D12";
    item.BgColor = "#FF0000";
    item.Url = "https://app.divoom-gz.com/User/GetUserData?SearchDivoomUser=400000012";
    item.RuleInfo = "n:FansCnt";
    item.RequestTime = 30;
    display_list.emplace_back(item);
  }

  // Item 10: Weather (Image with URL)
  {
    common::display_rendered item;
    item.type = common::display_type::Weather;  // Или Image, если это картинка погоды
    item.Type = "Weather";
    item.ID = 10;
    item.StartX = 33;
    item.StartY = 1135;
    item.Width = 64;
    item.Height = 64;
    item.Align = 2;
    item.FontSize = 115;
    item.FontID = 126;
    item.FontColor = "#484D12";
    item.BgColor = "#FF0000";
    item.Url = "https://f.divoom-gz.com/group1/M00/0C/4D/rBAAM2fZCOSEN4MoAAAAADuUrnI59.webp";
    display_list.emplace_back(item);
  }

  // Item 11: Temperature
  {
    common::display_rendered item;
    item.type = common::display_type::Temperature;
    item.Type = "Temperature";
    item.ID = 11;
    item.StartX = 18;
    item.StartY = 1199;
    item.Width = 93;
    item.Height = 33;
    item.Align = 2;
    item.FontSize = 25;
    item.FontID = 52;
    item.FontColor = "#CFAA63";
    item.BgColor = "#FF0000";
    display_list.emplace_back(item);
  }

  // Item 12: Image (Local file)
  {
    common::display_rendered item;
    item.type = common::display_type::Image;
    item.Type = "Image";
    item.ID = 12;
    item.StartX = 13;
    item.StartY = 227;
    item.Width = 320;
    item.Height = 320;
    item.Align = 2;
    item.FontSize = 115;
    item.FontID = 126;
    item.FontColor = "#484D12";
    item.BgColor = "#FF0000";
    item.Url = "/userdata/weather.gif";
    // Note: img_local_flag is not a field in struct, usually passed separately or ignored if URL is absolute
    display_list.emplace_back(item);
  }

  // Item 13: Text
  {
    common::display_rendered item;
    item.type = common::display_type::Text;
    item.Type = "Text";
    item.ID = 13;
    item.StartX = 0;
    item.StartY = 10;
    item.Width = 720;
    item.Height = 90;
    item.Align = 2;
    item.FontSize = 64;
    item.FontID = 126;
    item.FontColor = "#484D12";
    item.BgColor = "#FF0000";
    item.TextMessage = "this is text";
    display_list.emplace_back(item);
  }

  return display_list;
}

}  // namespace divoomdev::service