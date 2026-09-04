project(divoomdev.storage VERSION ${CMAKE_PROJECT_VERSION})

find_package(nlohmann_json CONFIG REQUIRED)
find_package(incbin CONFIG REQUIRED)
find_package(SQLite3 CONFIG REQUIRED)

# Абсолютный путь к каталогу SQL-ресурсов. Передаётся в src/sql_resources.cpp,
# где используется для встраивания скриптов через incbin (пути в incbin
# резолвятся относительно сборочной директории, поэтому требуется абсолютизация).
set(STORAGE_SQL_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

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
    incbin::incbin
    SQLite::SQLite3
    $<$<PLATFORM_ID:Windows>:bcrypt>  # for boost UUID
  INCLUDES_PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/mocs"
  TEST_SOURCES_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# add_target() не передаёт COMPILE_DEFINITIONS (баг в общем макросе), поэтому
# определяем путь к SQL-ресурсам напрямую для целевого модуля.
target_compile_definitions(${PROJECT_NAME} PRIVATE
  "STORAGE_SQL_DIR=\"${STORAGE_SQL_RESOURCES_DIR}\""
)

if (BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
