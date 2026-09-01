#pragma once
#include <gpch.h>

#ifndef PROJECT_NAME
#  define PROJECT_NAME "divoomdev"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "service"
#endif
#include <hare/hare.hpp>
namespace divoomdev::service {
MAKE_GETTER(log)
}  // namespace divoomdev::service

// External Libraries
#include <cpr/cpr.h>

#include <nlohmann/json.hpp>
