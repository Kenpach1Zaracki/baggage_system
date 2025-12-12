-- ============================================
-- Инициализация базы данных для системы багажа
-- ============================================

-- Создание таблицы для записей о багаже
CREATE TABLE IF NOT EXISTS baggage_records (
    id SERIAL PRIMARY KEY,
    flight_number VARCHAR(50) NOT NULL,
    passenger_name VARCHAR(255) NOT NULL,
    item_weights TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Создание индексов для ускорения поиска
CREATE INDEX IF NOT EXISTS idx_flight_number ON baggage_records(flight_number);
CREATE INDEX IF NOT EXISTS idx_passenger_name ON baggage_records(passenger_name);
CREATE INDEX IF NOT EXISTS idx_created_at ON baggage_records(created_at);

-- Комментарии к таблице и полям
COMMENT ON TABLE baggage_records IS 'Записи о багаже пассажиров';
COMMENT ON COLUMN baggage_records.id IS 'Уникальный идентификатор записи';
COMMENT ON COLUMN baggage_records.flight_number IS 'Номер рейса';
COMMENT ON COLUMN baggage_records.passenger_name IS 'Ф.И.О. пассажира';
COMMENT ON COLUMN baggage_records.item_weights IS 'Веса вещей (через запятую)';
COMMENT ON COLUMN baggage_records.created_at IS 'Дата и время создания записи';
COMMENT ON COLUMN baggage_records.updated_at IS 'Дата и время последнего обновления';

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

-- Тестовые данные для демонстрации
INSERT INTO baggage_records (flight_number, passenger_name, item_weights) VALUES
    ('SU1234', 'Иванов Иван Иванович', '15.50,22.00,18.30'),
    ('AF567', 'Петров Петр Петрович', '25.00'),
    ('BA890', 'Сидорова Мария Сергеевна', '12.50,30.00'),
    ('LH456', 'Козлов Андрей Викторович', '23.50'),
    ('AY789', 'Новикова Елена Дмитриевна', '10.00,15.00,20.00,12.50');

-- Вывод информации
DO $$
BEGIN
    RAISE NOTICE '✓ База данных багажной системы инициализирована успешно';
    RAISE NOTICE '✓ Создано записей: %', (SELECT COUNT(*) FROM baggage_records);
END $$;
