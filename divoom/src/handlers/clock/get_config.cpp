#include "get_config.h"

namespace divoomdev::divoom::handlers::clock {

namespace {
static int safe_get_int(const nlohmann::json* ptr, int default_val) {
  if (ptr && !ptr->is_null() && ptr->is_number()) {
    return ptr->get<int>();
  }
  return default_val;
}

// Вспомогательный метод для безопасного получения строки
static std::optional<std::string> safe_get_string(const nlohmann::json* ptr, const std::string& default_val) {
  if (ptr && !ptr->is_null() && ptr->is_string()) {
    return ptr->get<std::string>();
  }
  return default_val;
}

// Метод для парсинга одного элемента item
static clock_config::item_t parse_item(const nlohmann::json* item_ptr) {
  clock_config::item_t item;
  if (!item_ptr || item_ptr->is_null()) {
    return item;
  }

  if (item_ptr->contains("Invisible") && item_ptr->at("Invisible").is_number())
    item.Invisible = item_ptr->at("Invisible").get<int>();

  if (item_ptr->contains("ItemId") && item_ptr->at("ItemId").is_string())
    item.ItemId = item_ptr->at("ItemId").get<std::string>();

  if (item_ptr->contains("ItemName") && item_ptr->at("ItemName").is_string())
    item.ItemName = item_ptr->at("ItemName").get<std::string>();

  if (item_ptr->contains("ItemType") && item_ptr->at("ItemType").is_number())
    item.ItemType = item_ptr->at("ItemType").get<int>();

  if (item_ptr->contains("ItemValue") && item_ptr->at("ItemValue").is_string())
    item.ItemValue = item_ptr->at("ItemValue").get<std::string>();

  if (item_ptr->contains("ItemValueName") && item_ptr->at("ItemValueName").is_string())
    item.ItemValueName = item_ptr->at("ItemValueName").get<std::string>();

  if (item_ptr->contains("ShowByOtherItemId") && item_ptr->at("ShowByOtherItemId").is_string())
    item.ShowByOtherItemId = item_ptr->at("ShowByOtherItemId").get<std::string>();

  if (item_ptr->contains("ShowByOtherItemValue") && item_ptr->at("ShowByOtherItemValue").is_string())
    item.ShowByOtherItemValue = item_ptr->at("ShowByOtherItemValue").get<std::string>();

  if (item_ptr->contains("ControlOtherItemShow") && item_ptr->at("ControlOtherItemShow").is_array()) {
    for (const auto& val: item_ptr->at("ControlOtherItemShow")) {
      if (val.is_string()) {
        item.ControlOtherItemShow.push_back(val.get<std::string>());
      }
    }
  }
  return item;
}

static void parse_items_array(const nlohmann::json* array_ptr, std::vector<clock_config::item_t>& dest) {
  if (!array_ptr || !array_ptr->is_array()) {
    return;
  }

  for (const auto& item_json: *array_ptr) {
    dest.push_back(parse_item(&item_json));
  }
}

}  // namespace

get_config::get_config(int device_id, int clock_id, const user_info& user): handler(RequestType::POST) {
  request_.Command = "Channel/GetClockConfig";
  request_.DeviceId = device_id;
  request_.ClockId = clock_id;
  request_.UserId = user.UserId;
  request_.Token = user.Token;
}
get_config::~get_config() = default;

std::string get_config::get_path(const std::string_view host) const noexcept {
  return format("https://{}:443/Channel/GetClockConfig", host);
}

bool get_config::handle(const std::string& json_str) {
  result_ = std::nullopt;
  auto json = parse_json(json_str);

  if (!json) {
    return false;
  }

  try {
    clock_config config;

    // Парсинг опциональных строк
    config.AlbumShapePicId =
        safe_get_string(json->contains("AlbumShapePicId") ? &json->at("AlbumShapePicId") : nullptr, "");
    config.ClockExPlain = safe_get_string(json->contains("ClockExPlain") ? &json->at("ClockExPlain") : nullptr, "");

    // Парсинг опционального целого
    if (json->contains("AuthorUserId") && json->at("AuthorUserId").is_number()) {
      config.AuthorUserId = json->at("AuthorUserId").get<int>();
    }

    // Парсинг обязательных целых
    config.IsMyLike = safe_get_int(json->contains("IsMyLike") ? &json->at("IsMyLike") : nullptr, 0);
    config.LikeCnt = safe_get_int(json->contains("LikeCnt") ? &json->at("LikeCnt") : nullptr, 0);
    config.DeviceId = safe_get_int(json->contains("DeviceId") ? &json->at("DeviceId") : nullptr, 0);

    // Парсинг массивов
    if (json->contains("ItemList")) {
      parse_items_array(&json->at("ItemList"), config.ItemList);
    }

    if (json->contains("ItemList2")) {
      parse_items_array(&json->at("ItemList2"), config.ItemList2);
    }

    log()->debug("Successfully parsed clock config. ItemList size: {}, ItemList2 size: {}",
                 config.ItemList.size(),
                 config.ItemList2.size());
    result_ = config;
    return true;

  } catch (const std::exception& e) {
    log()->error("Failed to parse clock config: {}", e.what());
    return false;
  }
};

}  // namespace divoomdev::divoom::handlers::clock
