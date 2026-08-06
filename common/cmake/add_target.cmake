include_guard(GLOBAL)

function(add_target TARGET_NAME)
  set(options EXECUTABLE SHARED STATIC SHARED_OR_STATICOBJECT EXCLUDE_FROM_ALL)
  set(oneValueArgs INCLUDE_PATH SOURCES_PATH TEST_SOURCES_PATH)
  set(multiValueArgs 
    SOURCES USES_PUBLIC USES_PRIVATE INCLUDES_PUBLIC INCLUDES_PRIVATE
    PCH_PUBLIC PCH_PRIVATE
    TEST_SOURCES TEST_USES TEST_INCLUDES
    COMPILE_DEFINITIONS
  )
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (ARG_SOURCES_PATH)
    file(GLOB_RECURSE ARG_SOURCES_SEARCH_CPP "${ARG_SOURCES_PATH}/*.cpp")
    file(GLOB_RECURSE ARG_SOURCES_SEARCH_CC "${ARG_SOURCES_PATH}/*.cc")
    file(GLOB_RECURSE ARG_SOURCES_SEARCH_C "${ARG_SOURCES_PATH}/*.c")
    set(ARG_SOURCES ${ARG_SOURCES} ${ARG_SOURCES_SEARCH_CPP} ${ARG_SOURCES_SEARCH_CC} ${ARG_SOURCES_SEARCH_C})
  endif()

  if(NOT ARG_SOURCES)
    message(FATAL_ERROR "SOURCES or SOURCES_PATH argument is required for add_target")
  endif()

  if (ARG_EXCLUDE_FROM_ALL)
    set(EXCLUDE "EXCLUDE_FROM_ALL")
  else()
    set(EXCLUDE "")
  endif(ARG_EXCLUDE_FROM_ALL)

  if (ARG_TEST_SOURCES_PATH OR ARG_TEST_SOURCES)
    set(ADD_TESTS ON)
    if(ARG_EXECUTABLE OR ARG_SHARED)
      set(BUILD_TARGET_NAME ${TARGET_NAME}.objects)
    else()
      set(BUILD_TARGET_NAME ${TARGET_NAME})
    endif(ARG_EXECUTABLE OR ARG_SHARED)
  else()
    set(ADD_TESTS OFF)
    set(BUILD_TARGET_NAME ${TARGET_NAME})
  endif(ARG_TEST_SOURCES_PATH OR ARG_TEST_SOURCES)

  message(DEBUG "${TARGET_NAME} project of tests has files: ${ARG_SOURCES}")
  if (ARG_EXECUTABLE)
    if (ADD_TESTS)
      set(MAIN_SOURCE_FILE "")
      foreach(src ${ARG_SOURCES})
        get_filename_component(fname ${src} NAME)
        if(fname STREQUAL "main.cpp" OR 
           fname STREQUAL "main.cc" OR 
           fname STREQUAL "main.c")
            set(MAIN_SOURCE_FILE ${src})
            break()
        endif()
      endforeach()
      if(MAIN_SOURCE_FILE)
        list(REMOVE_ITEM ARG_SOURCES ${MAIN_SOURCE_FILE})
      else()
        message(WARNING 
          "Main source file NOT found for ${TARGET_NAME}. "
          "May be conflict with testing main function.")
      endif()

      add_library(${BUILD_TARGET_NAME} ${EXCLUDE_FROM_ALL} OBJECT ${ARG_SOURCES})
      add_executable(${TARGET_NAME} ${EXCLUDE_FROM_ALL} ${MAIN_SOURCE_FILE})
      target_link_libraries(${TARGET_NAME} PUBLIC ${BUILD_TARGET_NAME})
    else()
      add_executable(${TARGET_NAME} ${EXCLUDE_FROM_ALL} ${ARG_SOURCES})
    endif(ADD_TESTS)
  elseif (ARG_STATIC)
    add_library(${TARGET_NAME} ${EXCLUDE_FROM_ALL} STATIC ${ARG_SOURCES})
  elseif (ARG_SHARED)
    if (ADD_TESTS)
      add_library(${BUILD_TARGET_NAME} ${EXCLUDE_FROM_ALL} OBJECT ${ARG_SOURCES})
      add_library(${TARGET_NAME} ${EXCLUDE_FROM_ALL} SHARED)
      target_link_libraries(${TARGET_NAME} PUBLIC ${BUILD_TARGET_NAME})
    else()
      add_library(${TARGET_NAME} ${EXCLUDE_FROM_ALL} SHARED ${ARG_SOURCES})
    endif(ADD_TESTS)
  elseif (ARG_SHARED_OR_STATIC)
    message(FATAL "Unsupported yet!")
  elseif (ARG_OBJECT)
    add_library(${TARGET_NAME} ${EXCLUDE_FROM_ALL} OBJECT ${ARG_SOURCES})
  else()
    message(FATAL "Target type spicify error!")
  endif()

  target_link_libraries(${BUILD_TARGET_NAME} 
    PUBLIC ${ARG_USES_PUBLIC}
    PRIVATE ${ARG_USES_PRIVATE}
  )
  
  target_include_directories(${BUILD_TARGET_NAME}
    PUBLIC 
      ${ARG_INCLUDE_PATH}
      ${ARG_INCLUDES_PUBLIC}
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE 
      ${ARG_SOURCES_PATH}
      ${ARG_INCLUDES_PRIVATE}
  )

  target_precompile_headers(${BUILD_TARGET_NAME}
    PUBLIC ${ARG_PCH_PUBLIC}
    PRIVATE ${ARG_PCH_PRIVATE}
  )
  set_target_properties(${BUILD_TARGET_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
  )

  target_compile_definitions(${BUILD_TARGET_NAME} PRIVATE ${COMPILE_DEFINITIONS})
  
  if (BUILD_TESTS AND ADD_TESTS)
    setup_project_tests(
      TESTING_TARGET ${BUILD_TARGET_NAME}
      TARGET_NAME ${TARGET_NAME}.tests
      SOURCES_PATH ${ARG_TEST_SOURCES_PATH}
      SOURCES ${ARG_TEST_SOURCES}
      USES ${TEST_USES}
      INCLUDES ${ARG_SOURCES_PATH}
      COMPILE_DEFINITIONS ${COMPILE_DEFINITIONS}
    )
  endif(BUILD_TESTS AND ADD_TESTS)
endfunction()
