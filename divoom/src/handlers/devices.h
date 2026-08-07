#pragma once
#include "common/device.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers {

struct devices : handler<nullptr_t, std::vector<common::device>> {
  devices();
  ~devices() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers
