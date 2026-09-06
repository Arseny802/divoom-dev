#pragma once
#include <gpch.h>

#ifndef PROJECT_NAME
#  define PROJECT_NAME "divoomdev"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "jira"
#endif
#include <hare/hare.hpp>
namespace divoomdev::jira {
MAKE_GETTER(log)
}  // namespace divoomdev::jira
