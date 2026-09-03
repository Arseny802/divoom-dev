#pragma once
#include "common/font.h"
#include "handler.hpp"

namespace divoomdev::divoom::handlers {

struct font_list : handler<nullptr_t, common::font_list> {
  font_list();
  ~font_list() override;

  std::string get_path(const std::string_view host) const noexcept override;
  bool handle(const std::string& data) override;
};

}  // namespace divoomdev::divoom::handlers
