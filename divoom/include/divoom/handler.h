#pragma once
#include "common/common.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/variant.hpp>

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
  std::optional<ResponseT> result_;
  RequestDataT request_;
};
}  // namespace divoomdev::divoom
