#pragma once
#include <memory>
#include <string>

#include <hare/hlogger.h>

#include "storage/backend.h"
#include "storage/i_storage.h"

namespace divoomdev::storage {

hare::hlogger_ptr get_logger();

/// Открывает хранилище настроек на выбранном бэкенде.
///
/// \param type  Тип бэкенда: sqlite (БД) или json_file (config.json).
/// \param path  Путь к файлу БД (*.db / *.sqlite) либо к config.json.
///              Для пустой/несуществующей БД будет применена базовая схема и
///              миграции (скрипты встроены через incbin).
/// \return      Реализация `i_settings_storage` либо `nullptr` при ошибке
///              открытия/миграции.
std::unique_ptr<i_settings_storage> open_storage(backend_type type, const std::string& path);

}  // namespace divoomdev::storage