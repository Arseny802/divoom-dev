project(divoomdev.storage VERSION ${CMAKE_PROJECT_VERSION})

find_package(nlohmann_json CONFIG REQUIRED)

# file(GLOB SQLite_SOURCES "sql/sqlite/*.sql")
# cmrc_add_resource_library(${PROJECT_NAME}.resources
#   NAMESPACE ymsummorizer::storage::sql_resources
#   WHENCE sql
#   ${SQLite_SOURCES}
# )

add_target(${PROJECT_NAME} STATIC
  INCLUDE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PCH_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
  USES_PUBLIC 
    divoomdev.common
    #${PROJECT_NAME}.resources
    nlohmann_json::nlohmann_json
    #SQLite::SQLite3
    $<$<PLATFORM_ID:Windows>:bcrypt>  # for boost UUID
  INCLUDES_PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/mocs"
  TEST_SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

if (BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
