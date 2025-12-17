#!/bin/bash
# ============================================
# Скрипт для локального запуска приложения
# ============================================

echo "🚀 Запуск системы управления багажом (локально)..."

# Проверка наличия собранного приложения
if [ ! -f "./build/BaggageSystem" ]; then
    echo "❌ Приложение не собрано! Запустите сборку:"
    echo "   cd build && cmake .. && make"
    exit 1
fi

# Проверка PostgreSQL
if ! docker-compose ps | grep -q "baggage_postgres.*Up"; then
    echo "⚠️  PostgreSQL не запущен. Запускаю контейнер..."
    docker-compose up -d postgres
    echo "⏳ Ожидание запуска PostgreSQL..."
    sleep 3
fi

# Проверка X11 (для GUI)
if [ -z "$DISPLAY" ]; then
    echo "⚠️  DISPLAY не установлен, использую :0"
    export DISPLAY=:0
fi

# Экспорт переменных окружения для подключения к БД
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=baggage_db
export DB_USER=postgres
export DB_PASSWORD=postgres

echo "✅ Параметры подключения:"
echo "   Host: $DB_HOST:$DB_PORT"
echo "   Database: $DB_NAME"
echo "   User: $DB_USER"
echo ""
echo "🔑 Для входа используйте:"
echo "   Логин: admin"
echo "   Пароль: admin"
echo ""

# Запуск приложения
./build/BaggageSystem

echo "✅ Приложение завершено"
