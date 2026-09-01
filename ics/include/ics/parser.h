#pragma once
#include "common/common.hpp"
#include <memory>
#include <string>

namespace divoomdev::ics {

class parser final {
 public:
  struct options {
    bool ignore_repeat = false;
  };

  parser(options opt);
  ~parser();

  bool parse(const std::string& ics_data);

 protected:
  options options_;
};

}  // namespace divoomdev::ics
