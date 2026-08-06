#pragma once
#include "common/common_pch.h"

#ifndef PROJECT_NAME
#  define PROJECT_NAME "divoomdev"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "storage.example"
#endif
#include <hare/hare.hpp>
namespace divoomdev::storage::example {
MAKE_GETTER(log)
}  // namespace divoomdev::storage::example
