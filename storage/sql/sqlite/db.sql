-- ============================================================================
-- divoomdev.storage — базовая схема v1.
-- ----------------------------------------------------------------------------
-- Этот файл применяется миграционным раннером один раз при первом открытии
-- БД (когда PRAGMA user_version == 0). Он создаёт таблицы и фиксирует версию
-- схемы в `meta` и в `PRAGMA user_version`.
--
-- Все последующие изменения схемы должны оформляться отдельными файлами в
-- каталоге storage/sql/sqlite/migrations/NNNN_<name>.sql и регистрироваться
-- в src/sql_resources.cpp. Каждый файл миграции обязан в конце обновить
-- `PRAGMA user_version`.
--
-- Версия схемы, создаваемая этим файлом: 1
-- ============================================================================

PRAGMA foreign_keys = ON;

-- Метаданные хранилища (ключ-значение): schema_version, а в будущем и другие
-- служебные данные (например, master-ключ шифрования).
CREATE TABLE IF NOT EXISTS meta (
    key   TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

-- Учётные записи онлайн-сервисов. Колонка `service` делает таблицу
-- масштабируемой: сегодня хранится один сервис (divoom), завтра — произвольное
-- множество без изменения схемы.
CREATE TABLE IF NOT EXISTS accounts (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    service    TEXT NOT NULL UNIQUE,
    login      TEXT NOT NULL,
    password   TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now')),
    updated_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now'))
);

-- ICS-календари: список URL с приоритетом загрузки и флагом включения.
-- Более высокий `priority` означает более высокий приоритет (меньшее число
-- загружается раньше).
CREATE TABLE IF NOT EXISTS calendar_sources (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    url        TEXT NOT NULL UNIQUE,
    priority   INTEGER NOT NULL DEFAULT 0,
    enabled    INTEGER NOT NULL DEFAULT 1,
    created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now'))
);

INSERT INTO meta (key, value) VALUES ('schema_version', '1')
    ON CONFLICT(key) DO NOTHING;

PRAGMA user_version = 1;