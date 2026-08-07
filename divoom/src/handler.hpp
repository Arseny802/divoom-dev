#pragma once
#include "common/common.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace divoomdev::divoom {

enum RequestType {
  GET,
  POST,
};

template<typename RequestDataT = nullptr_t, typename ResponseT = nullptr_t>
struct handler {
  using RequestDataType = RequestDataT;
  using ResultType = ResponseT;

  handler(RequestType type): request_type(type) { }
  virtual ~handler() = default;

  std::string get_request() {
    if constexpr (std::is_same<decltype(request_), nullptr_t>::value) {
      return {};
    }
    return nlohmann::json(request_).dump();
  }

  ResponseT get_result() {
    if (!result_) {
      throw std::runtime_error("No result available");
    }
    ResponseT temp = *result_;
    result_ = std::nullopt;
    return temp;
  };

  virtual std::string get_path(const std::string_view host) const noexcept = 0;
  virtual void set_request() { };
  virtual bool handle(const std::string&) = 0;

  const RequestType request_type = RequestType::GET;

 protected:
  std::unique_ptr<nlohmann::json> parse_json(const std::string& data);

  std::optional<ResponseT> result_;
  RequestDataT request_;
};

template<typename T1, typename T2>
std::unique_ptr<nlohmann::json> handler<T1, T2>::parse_json(const std::string& data) {
  std::unique_ptr<nlohmann::json> j;
  try {
    j = std::make_unique<nlohmann::json>(nlohmann::json::parse(data));
  } catch (const nlohmann::json::parse_error& e) {
    log()->error("JSON parse error: {}", e.what());
    return nullptr;
  }

  if ((*j)["ReturnCode"] != 0) {
    log()->error("API error: {}", j->value("ReturnMessage", "Unknown error"));
    return nullptr;
  }
  return j;
}

}  // namespace divoomdev::divoom
