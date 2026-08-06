include_guard(GLOBAL)

function(add_version_header)
  set(options)
  set(oneValueArgs PROJECT_DEFINITION HEADER_DIR_PATH)
  set(multiValueArgs)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

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
endfunction()
