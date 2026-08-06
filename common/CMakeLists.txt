project(divoomdev.common VERSION ${CMAKE_PROJECT_VERSION})

find_package(magic_enum REQUIRED)
find_package(Boost REQUIRED)

add_target(${PROJECT_NAME} STATIC
  INCLUDE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PCH_PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common/common_pch.h
  PCH_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
  USES_PUBLIC 
    hare
    magic_enum::magic_enum
    Boost::boost
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Windows>:wsock32>
  TEST_SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

configure_file(version.h.in 
  ${PROJECT_SOURCE_DIR}/include/common/version.current.h
  NEWLINE_STYLE LF
)

execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files 
  ${PROJECT_SOURCE_DIR}/include/common/version.current.h
  ${PROJECT_SOURCE_DIR}/include/common/version.h
  RESULT_VARIABLE VersionChanged
)
if(VersionChanged EQUAL 0)
  message(STATUS "Version was NOT changed")
elseif(VersionChanged EQUAL 1)
  message(STATUS "Project version changed - updating version.h")
  file(REMOVE ${PROJECT_SOURCE_DIR}/include/common/version.h)
  file(RENAME 
    ${PROJECT_SOURCE_DIR}/include/common/version.current.h
    ${PROJECT_SOURCE_DIR}/include/common/version.h 
  )
else()
  message(WARNING "Error while comparing version.h files!")
endif()

