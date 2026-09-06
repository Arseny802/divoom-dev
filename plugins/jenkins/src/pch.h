#pragma once
#include <gpch.h>

#ifndef PROJECT_NAME
#  define PROJECT_NAME "divoomdev"
#endif
#ifndef MODULE_NAME
#  define MODULE_NAME "jenkins"
#endif
#include <hare/hare.hpp>
namespace divoomdev::jenkins {
MAKE_GETTER(log)
}  // namespace divoomdev::jenkins
