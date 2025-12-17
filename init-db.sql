-- ============================================
-- Инициализация базы данных для системы багажа
-- ============================================

-- Создание таблицы для записей о багаже
CREATE TABLE IF NOT EXISTS baggage_records (
    id SERIAL PRIMARY KEY,
    flight_number VARCHAR(50) NOT NULL,
    passenger_name VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by INTEGER -- ID пользователя, создавшего запись
);

-- Создание таблицы для отдельных вещей (НОРМАЛИЗАЦИЯ)
CREATE TABLE IF NOT EXISTS baggage_items (
    id SERIAL PRIMARY KEY,
    baggage_record_id INTEGER NOT NULL REFERENCES baggage_records(id) ON DELETE CASCADE,
    item_number INTEGER NOT NULL CHECK (item_number >= 1 AND item_number <= 5),
    weight NUMERIC(5,2) NOT NULL CHECK (weight > 0 AND weight <= 100),
    UNIQUE(baggage_record_id, item_number)
);

-- Создание индексов для ускорения поиска
CREATE INDEX IF NOT EXISTS idx_flight_number ON baggage_records(flight_number);
CREATE INDEX IF NOT EXISTS idx_passenger_name ON baggage_records(passenger_name);
CREATE INDEX IF NOT EXISTS idx_created_at ON baggage_records(created_at);
CREATE INDEX IF NOT EXISTS idx_baggage_items_record ON baggage_items(baggage_record_id);

-- Комментарии к таблице и полям
COMMENT ON TABLE baggage_records IS 'Записи о багаже пассажиров';
COMMENT ON COLUMN baggage_records.id IS 'Уникальный идентификатор записи';
COMMENT ON COLUMN baggage_records.flight_number IS 'Номер рейса';
COMMENT ON COLUMN baggage_records.passenger_name IS 'Ф.И.О. пассажира';
COMMENT ON COLUMN baggage_records.created_at IS 'Дата и время создания записи';
COMMENT ON COLUMN baggage_records.updated_at IS 'Дата и время последнего обновления';
COMMENT ON COLUMN baggage_records.created_by IS 'ID пользователя, создавшего запись';

COMMENT ON TABLE baggage_items IS 'Отдельные вещи багажа (нормализованная структура)';
COMMENT ON COLUMN baggage_items.id IS 'Уникальный идентификатор вещи';
COMMENT ON COLUMN baggage_items.baggage_record_id IS 'Ссылка на запись багажа';
COMMENT ON COLUMN baggage_items.item_number IS 'Номер вещи (1-5)';
COMMENT ON COLUMN baggage_items.weight IS 'Вес вещи в кг (0.01-100.00)';

-- Тригер для автоматического обновления updated_at
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_baggage_records_updated_at
    BEFORE UPDATE ON baggage_records
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Создание таблицы пользователей (ТЗ п. 1.2.8.4.2, п. 1.2.4.5.6)
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(64) NOT NULL,
    role VARCHAR(20) DEFAULT 'user',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Создание индекса для быстрого поиска по username
CREATE INDEX IF NOT EXISTS idx_username ON users(username);

-- Комментарии к таблице users
COMMENT ON TABLE users IS 'Пользователи системы с контролем доступа';
COMMENT ON COLUMN users.id IS 'Уникальный идентификатор пользователя';
COMMENT ON COLUMN users.username IS 'Имя пользователя (логин)';
COMMENT ON COLUMN users.password_hash IS 'SHA-256 хеш пароля';
COMMENT ON COLUMN users.role IS 'Роль пользователя (admin/user)';
COMMENT ON COLUMN users.created_at IS 'Дата регистрации пользователя';

-- Создание администратора по умолчанию
-- Логин: admin
INSERT INTO users (username, password_hash, role)
VALUES ('admin', '286057e1642b4a482258fa71c57d7c700ba55fa8b6b15f5593648c800a1dad02', 'admin')
ON CONFLICT (username) DO NOTHING;

-- Тестовые данные для демонстрации
-- Сначала вставляем записи багажа
INSERT INTO baggage_records (id, flight_number, passenger_name) VALUES
    (1, 'SU1234', 'Иванов Иван Иванович'),
    (2, 'AF567', 'Петров Петр Петрович'),
    (3, 'BA890', 'Сидорова Мария Сергеевна'),
    (4, 'LH456', 'Козлов Андрей Викторович'),
    (5, 'AY789', 'Новикова Елена Дмитриевна')
ON CONFLICT (id) DO NOTHING;

-- Теперь вставляем вещи для каждой записи
INSERT INTO baggage_items (baggage_record_id, item_number, weight) VALUES
    -- Иванов (3 вещи)
    (1, 1, 15.50),
    (1, 2, 22.00),
    (1, 3, 18.30),
    -- Петров (1 вещь)
    (2, 1, 25.00),
    -- Сидорова (2 вещи)
    (3, 1, 12.50),
    (3, 2, 30.00),
    -- Козлов (1 вещь)
    (4, 1, 23.50),
    -- Новикова (4 вещи)
    (5, 1, 10.00),
    (5, 2, 15.00),
    (5, 3, 20.00),
    (5, 4, 12.50)
ON CONFLICT (baggage_record_id, item_number) DO NOTHING;

-- Обновляем счетчик последовательности
SELECT setval('baggage_records_id_seq', (SELECT MAX(id) FROM baggage_records));

-- Вывод информации
DO $$
BEGIN
    RAISE NOTICE '========================================';
    RAISE NOTICE '✓ База данных багажной системы инициализирована';
    RAISE NOTICE '✓ Создано записей багажа: %', (SELECT COUNT(*) FROM baggage_records);
    RAISE NOTICE '✓ Создано вещей: %', (SELECT COUNT(*) FROM baggage_items);
    RAISE NOTICE '✓ Создано пользователей: %', (SELECT COUNT(*) FROM users);
    RAISE NOTICE '========================================';
    RAISE NOTICE '  Доступ администратора:';
    RAISE NOTICE '  Логин: admin';
    RAISE NOTICE '  Пароль: poprobuy_vzlomat15';
    RAISE NOTICE '========================================';
    RAISE NOTICE '  Структура БД обновлена:';
    RAISE NOTICE '  - Таблица baggage_items (нормализация)';
    RAISE NOTICE '  - Проверки целостности на уровне БД';
    RAISE NOTICE '  - Каскадное удаление связанных записей';
    RAISE NOTICE '========================================';
END $$;
