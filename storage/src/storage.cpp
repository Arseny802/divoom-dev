#include "pch.h"

namespace divoomdev::storage {

hare::hlogger_ptr get_logger() {
  return log();
}
}  // namespace divoomdev::storage
