cmake_minimum_required(VERSION 3.23)
project(divoomdev VERSION 0.0.1)

add_subdirectory(deps)
add_subdirectory(common)
add_subdirectory(divoom)
add_subdirectory(storage)
add_subdirectory(plugins)
add_subdirectory(service)

set(INSTALL_DESTINATION divoomdev)
install(
  TARGETS divoomdev.service
  DESTINATION ${INSTALL_DESTINATION}
)
