#pragma once
#include <gpch.h>

#ifndef PROJECT_NAME
#  define PROJECT_NAME "divoomdev"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "divoom"
#endif
#include <hare/hare.hpp>
namespace divoomdev::divoom {
MAKE_GETTER(log)
}  // namespace divoomdev::divoom

// External Libraries
#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

#include <magic_enum.hpp>
