# Файл для настройки генерации отчётов о покрытии тестами через gcov/lcov
# Использование:
# include(common_coverage)
# enable_coverage()

include_guard(GLOBAL)

if(NOT ENABLE_COVERAGE)
  return()
endif()

# Проверяем, поддерживается ли покрытие на текущей платформе
if(NOT CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(WARNING "Code coverage requires GCC or Clang. Coverage disabled.")
  function(setup_coverage_targets)
    message(WARNING "setup_coverage_targets() unavailable for MSVC")
  endfunction()
  return()
endif()

# Добавляем флаги для генерации данных о покрытии
add_compile_options(--coverage)
# Для старых версий GCC
# add_compile_options(-fprofile-arcs -ftest-coverage)

# Добавляем флаги линковки
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
# Для старых версий GCC
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs")

# Находим утилиты gcov и lcov
find_program(GCOV_PATH gcov)
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

if(NOT GCOV_PATH)
  message(WARNING "gcov не найден! Покрытие будет недоступно.")
endif()

if(NOT LCOV_PATH)
  message(WARNING "lcov не найден! Сбор отчётов будет ограничен.")
endif()

if(NOT GENHTML_PATH)
  message(WARNING "genhtml не найден! Генерация HTML отчётов будет недоступна.")
endif()

set(COVERAGE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/coverage)
file(MAKE_DIRECTORY ${COVERAGE_OUTPUT_DIR})

# Функция для генерации отчёта о покрытии
function(generate_coverage_report)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs TARGETS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Имя отчёта
  if(NOT ARG_NAME)
    set(ARG_NAME "coverage")
  endif()

  set(COVERAGE_TARGET ${ARG_NAME}.coverage)

  # Цель для генерации отчёта
  add_custom_target(${COVERAGE_TARGET}
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}"
    
    # Генерируем .gcov файлы
    COMMAND ${GCOV_PATH} --relative-only -pb "${CMAKE_BINARY_DIR}"
    
    # Если доступен lcov, создаём .info файл
    ${LCOV_PATH} "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}.info" --capture --directory "${CMAKE_BINARY_DIR}" --output-file "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}.info"
    
    # Фильтруем данные (удаляем системные и внешние файлы)
    ${LCOV_PATH} "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}.info" --remove "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}.info" "*/deps/*" "*/test/*" "*/tests/*" "*/external/*" "*/third_party/*" "*/usr/*" "*/usr/include/*" "/lib/*" "*/include/*" --output-file "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}.filtered.info"
    
    # Генерируем HTML отчёт
    ${GENHTML_PATH} "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}.filtered.info" --output-directory "${COVERAGE_OUTPUT_DIR}/${ARG_NAME}"
    
    # Выводим путь к отчёту
    COMMAND ${CMAKE_COMMAND} -E echo "Отчёт о покрытии доступен по адресу: file:///${COVERAGE_OUTPUT_DIR}/${ARG_NAME}/index.html"
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Генерация отчёта о покрытии ${ARG_NAME}..."
    VERBATIM
  )

  # Добавляем зависимость от выполнения тестов
  add_dependencies(${COVERAGE_TARGET} test)

  message(STATUS "Настроена генерация отчёта о покрытии: ${COVERAGE_TARGET}")
endfunction()

# Функция для настройки покрытия для тестов
function(setup_coverage_targets)
  # Создаём общую цель для покрытия
  generate_coverage_report(NAME "all")
endfunction()
