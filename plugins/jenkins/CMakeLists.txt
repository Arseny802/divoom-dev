project(divoomdev.jenkins VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

add_target(${PROJECT_NAME} STATIC
  INCLUDE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PCH_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
  USES_PUBLIC 
    divoomdev.common
    nlohmann_json::nlohmann_json
    cpr::cpr
  TEST_SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
