#pragma once
#include <optional>
#include <string>

namespace divoomdev::divoom {
struct clock_config {
  std::optional<std::string> AlbumShapePicId = std::nullopt;
  std::optional<int> AuthorUserId = std::nullopt;
  std::optional<std::string> ClockExPlain = std::nullopt;
  int IsMyLike{};
  int LikeCnt{};

  int DeviceId{};

  struct item_t {
    int Invisible;
    std::string ItemId;
    std::string ItemName;
    int ItemType;
    std::string ItemValue;
    std::string ItemValueName;
    std::string ShowByOtherItemId;
    std::string ShowByOtherItemValue;
    std::vector<std::string> ControlOtherItemShow;
  };
  using item_list_t = std::vector<item_t>;

  item_list_t ItemList{};
  item_list_t ItemList2{};
};

#ifdef NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(clock_config::item_t,
                                   Invisible,
                                   ItemId,
                                   ItemName,
                                   ItemType,
                                   ItemValue,
                                   ItemValueName,
                                   ShowByOtherItemId,
                                   ShowByOtherItemValue,
                                   ControlOtherItemShow);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(clock_config,
                                   // AlbumShapePicId,
                                   // AuthorUserId,
                                   // ClockExPlain,
                                   IsMyLike,
                                   LikeCnt,
                                   DeviceId,
                                   ItemList,
                                   ItemList2);
#endif
}  // namespace divoomdev::divoom
