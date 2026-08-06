cmake_minimum_required(VERSION 3.23)
project(divoomdev.storage.example VERSION 0.0.1)

add_target(${PROJECT_NAME} EXECUTABLE
  SOURCES_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PCH_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
  USES_PUBLIC 
    divoomdev.storage
)
