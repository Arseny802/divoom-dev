#pragma once
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "divoom/exceptions.h"
#include "status_codes.h"

namespace divoomdev::divoom {

enum RequestType {
  GET,
  POST,
};

template<typename RequestDataT = nullptr_t, typename ResponseT = nullptr_t>
struct handler {
  using RequestDataType = RequestDataT;
  using ResultType = ResponseT;

  handler(RequestType type);
  virtual ~handler();

  std::string get_request();
  ResponseT get_result();

  virtual std::string get_path(const std::string_view host) const noexcept = 0;
  virtual bool handle(const std::string& data);

  const RequestType request_type = RequestType::GET;

 protected:
  std::unique_ptr<nlohmann::json> parse_json(const std::string& data);

  std::optional<ResponseT> result_;
  RequestDataT request_;
};

template<typename T1, typename T2>
handler<T1, T2>::handler(RequestType type): request_type(type) { }

template<typename T1, typename T2>
handler<T1, T2>::~handler() = default;

template<typename T1, typename T2>
std::string handler<T1, T2>::get_request() {
  if constexpr (std::is_same<decltype(request_), nullptr_t>::value) {
    return {};
  }
  return nlohmann::json(request_).dump();
}

template<typename T1, typename T2>
handler<T1, T2>::ResultType handler<T1, T2>::get_result() {
  if (!result_) {
    throw std::runtime_error("No result available");
  }
  ResultType temp = *result_;
  result_ = std::nullopt;
  return temp;
};

template<typename T1, typename T2>
bool handler<T1, T2>::handle(const std::string& data) {
  return parse_json(data).get();
}

template<typename T1, typename T2>
std::unique_ptr<nlohmann::json> handler<T1, T2>::parse_json(const std::string& data) {
  std::unique_ptr<nlohmann::json> j;
  try {
    j = std::make_unique<nlohmann::json>(nlohmann::json::parse(data));
  } catch (const nlohmann::json::parse_error& e) {
    log()->error("JSON parse error: {}", e.what());
    return nullptr;
  }

  const auto status_code = j->at("ReturnCode").get<int>();
  switch (status_code) {
  case ErrorCode::OK: break;
  case ErrorCode::HTTP_NORMAL_ERROR:
  case ErrorCode::HTTP_REGISTER_ERROR1:
  case ErrorCode::HTTP_REGISTER_ERROR2:
  case ErrorCode::HTTP_ADD_BUDDY_ERROR:
  case ErrorCode::HTTP_GET_ERROR:
  case ErrorCode::HTTP_ERROR_CAN_NOT_MATCH:
  case ErrorCode::HTTP_ERROR_CAN_NOT_DEAL_WITH:
  case ErrorCode::HTTP_ERROR_WRONG_CMD:
  case ErrorCode::HTTP_REQUEST_EMPTY:
  case ErrorCode::HTTP_REQUEST_JSON_ERROR:
  case ErrorCode::HTTP_BUDDY_HAD_FRIEND:
  case ErrorCode::HTTP_HAD_NOT_FRIEND:
  case ErrorCode::HTTP_GALLERY_UPLOAD_ERROR:
  case ErrorCode::HTTP_PHONE_FORMAT_ERROR:
  case ErrorCode::HTTP_PHONE_CHECK_ERROR:
  case ErrorCode::HTTP_TOO_MATCH:
  case ErrorCode::HTTP_NEED_CHECK:
  case ErrorCode::BLACK_ERROR:
  case ErrorCode::LIMIT_UPLOAD:
  case ErrorCode::BLUETOOTH_PASSWORD_ERROR:
  case ErrorCode::HTTP_LOCK_ACCOUNT:
  case ErrorCode::HTTP_UPLOAD_TEXT_TO_SERVER:
    {
      auto code_name = magic_enum::enum_name(static_cast<ErrorCode>(status_code));
      log()->error("API error '{}': {}", code_name, j->value("ReturnMessage", "Unknown error"));
      return nullptr;
    }
  case ErrorCode::HTTP_LOGIN_ERROR_NO_USER: throw WrongUserException();
  case ErrorCode::HTTP_LOGIN_ERROR_PASSWORD: throw WrongPasswordException();
  case ErrorCode::HTTP_ERROR_TOKEN_MISSMATCH: throw TokenExpiredException();
  case ErrorCode::FORBIDEN_ERROR: throw TokenExpiredException();
  default:
    {
      log()->error("API error: {}", j->value("ReturnMessage", "Unknown error"));
      return nullptr;
    }
  }

  return j;
}

}  // namespace divoomdev::divoom
