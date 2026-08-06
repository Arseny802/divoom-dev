# Файл для инициализации unit-тестов в проектах ymsummorizer
# Использование:
# include(common_tests)
# setup_project_tests(
#   NAME test_project_name
#   SOURCES test1.cpp test2.cpp
# )
include_guard(GLOBAL)

if (NOT BUILD_TESTS)
  function(setup_project_tests)
    set(options)
    set(oneValueArgs TESTING_TARGET)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT ARG_TESTING_TARGET)
      set(ARG_TESTING_TARGET ${PROJECT_NAME})
    endif(NOT ARG_TESTING_TARGET)
    
    message(STATUS "$Tests for {ARG_TESTING_TARGET} will not be generated - BUILD_TESTS is off.")
  endfunction()
  return()
endif(NOT BUILD_TESTS)

include(CTest)

find_package(GTest REQUIRED)

function(setup_project_tests)
  set(options)
  set(oneValueArgs TESTING_TARGET TARGET_NAME SOURCES_PATH)
  set(multiValueArgs SOURCES USES INCLUDES COMPILE_DEFINITIONS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if (NOT ARG_TESTING_TARGET)
    set(ARG_TESTING_TARGET ${PROJECT_NAME})
  endif(NOT ARG_TESTING_TARGET)

  if(NOT ARG_TARGET_NAME)
    set(ARG_TARGET_NAME ${ARG_TESTING_TARGET}.test)
  endif()

  if(ARG_SOURCES_PATH)
    file(GLOB_RECURSE ARG_SOURCES_SEARCH_CPP "${ARG_SOURCES_PATH}/*.cpp")
    file(GLOB_RECURSE ARG_SOURCES_SEARCH_CC "${ARG_SOURCES_PATH}/*.cc")
    file(GLOB_RECURSE ARG_SOURCES_SEARCH_C "${ARG_SOURCES_PATH}/*.c")
    set(ARG_SOURCES ${ARG_SOURCES} ${ARG_SOURCES_SEARCH_CPP} ${ARG_SOURCES_SEARCH_CC} ${ARG_SOURCES_SEARCH_C})
  endif()

  if(NOT ARG_SOURCES)
    message(FATAL_ERROR "SOURCES or SOURCES_PATH argument is required for setup_project_tests")
  endif()

  message(DEBUG "${ARG_TARGET_NAME} project of tests has files: ${ARG_SOURCES}")

  add_executable(${ARG_TARGET_NAME} ${ARG_SOURCES})
  target_link_libraries(${ARG_TARGET_NAME} PRIVATE
    ${ARG_TESTING_TARGET}
    ${ARG_USES}
    GTest::gmock_main
  )

  set_target_properties(${ARG_TARGET_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
  )

  target_compile_definitions(${ARG_TARGET_NAME} PRIVATE UNIT_TEST)
  target_include_directories(${ARG_TARGET_NAME} PRIVATE ${ARG_INCLUDES} ${ARG_SOURCES_PATH})

  # Регистрация теста в CTest
  add_test(NAME ${ARG_TARGET_NAME} COMMAND ${ARG_TARGET_NAME})
  message(STATUS "Configured test: ${ARG_TARGET_NAME}")

  if (RUN_TESTS)
    add_custom_command(
      TARGET ${ARG_TARGET_NAME}
      POST_BUILD
      COMMAND ctest -C $<CONFIGURATION> --output-on-failure)
    if (ENABLE_COVERAGE AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
      generate_coverage_report(NAME ${ARG_TARGET_NAME})
    endif()
  else()
    message(STATUS "Test run disabled - ${ARG_NAME} tests will not ve created and executed.")
  endif(RUN_TESTS)
endfunction()
