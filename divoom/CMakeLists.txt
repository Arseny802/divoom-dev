project(divoomdev.divoom VERSION ${CMAKE_PROJECT_VERSION})

find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(mbedtls CONFIG REQUIRED)

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
    MbedTLS::mbedtls
  TEST_SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
