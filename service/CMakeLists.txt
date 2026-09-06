project(divoomdev.service VERSION ${CMAKE_PROJECT_VERSION})

add_target(${PROJECT_NAME} EXECUTABLE
  SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PCH_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
  USES_PUBLIC 
    divoomdev.common
    divoomdev.divoom
    divoomdev.storage
    divoomdev.ics
  TEST_SOURCES_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
  TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../storage/mocs/dbmoc.hpp
  TEST_USES
    divoomdev.divoom
    divoomdev.ics
    divoomdev.storage
  TEST_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../storage/mocs/
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  OUTPUT_NAME "divoomdev-${PROJECT_VERSION}" 
)
