cmake_minimum_required(VERSION 3.23)
project(divoomdev VERSION 0.0.1)

option(BUILD_TESTS "Build project tests" ON)
option(RUN_TESTS "Run project tests" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" ON)
option(BUILD_EXAMPLES "Build project examples" ON)

include(common/common.cmake)

add_subdirectory(deps)
add_subdirectory(common)
add_subdirectory(divoom)
add_subdirectory(storage)
add_subdirectory(service)

set(INSTALL_DESTINATION divoomdev)
install(
  TARGETS divoomdev.service
  DESTINATION ${INSTALL_DESTINATION}
)
