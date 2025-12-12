# Запуск приложения в Docker

Система управления багажом упакована в Docker контейнеры для удобного развертывания.

## Требования

- **Docker** версии 20.10 или выше
- **Docker Compose** версии 1.29 или выше
- **X11** для отображения GUI (Linux/macOS)

## Быстрый старт

### 1. Разрешите X11 подключения (только для Linux/macOS)

```bash
# Разрешить подключения к X server
xhost +local:docker
```

### 2. Запустите приложение

```bash
# Сборка и запуск всех сервисов
docker-compose up --build
```

Приложение автоматически:
- Создаст PostgreSQL базу данных
- Инициализирует таблицы
- Добавит тестовые данные
- Запустит GUI приложение

### 3. Остановка приложения

```bash
# Остановить контейнеры
docker-compose down

# Остановить и удалить тома (⚠️ удалит все данные)
docker-compose down -v
```

## Архитектура

Проект состоит из двух сервисов:

### 1. PostgreSQL (postgres)
- **Образ**: `postgres:15-alpine`
- **Порт**: 5432
- **База данных**: baggage_db
- **Пользователь**: postgres
- **Пароль**: postgres

### 2. Qt Приложение (baggage_app)
- **Сборка**: Multi-stage build (Ubuntu 22.04)
- **Зависимости**: Qt 6, PostgreSQL драйвер
- **Network mode**: host (для доступа к X11)

## Переменные окружения

Можно изменить параметры подключения к БД в `docker-compose.yml`:

```yaml
environment:
  DB_HOST: postgres
  DB_PORT: 5432
  DB_NAME: baggage_db
  DB_USER: postgres
  DB_PASSWORD: postgres
```

## Полезные команды

### Просмотр логов

```bash
# Все сервисы
docker-compose logs -f

# Только БД
docker-compose logs -f postgres

# Только приложение
docker-compose logs -f baggage_app
```

### Подключение к PostgreSQL

```bash
# Через psql
docker-compose exec postgres psql -U postgres -d baggage_db

# Список всех записей
docker-compose exec postgres psql -U postgres -d baggage_db -c "SELECT * FROM baggage_records;"
```

### Пересборка образа

```bash
# Пересобрать образ приложения
docker-compose build --no-cache baggage_app

# Пересобрать все образы
docker-compose build --no-cache
```

### Очистка

```bash
# Удалить все контейнеры и тома
docker-compose down -v

# Удалить образ приложения
docker rmi baggage_system_baggage_app

# Полная очистка Docker (⚠️ удалит ВСЕ неиспользуемые образы и тома)
docker system prune -a --volumes
```

## Только база данных (без GUI)

Если нужна только БД для разработки:

```bash
# Запустить только PostgreSQL
docker-compose up postgres

# Приложение запускается локально
./build/BaggageSystem
```

## Проблемы и решения

### GUI не отображается

**Проблема**: Окно не появляется

**Решение**:
```bash
# 1. Проверьте DISPLAY
echo $DISPLAY

# 2. Разрешите X11
xhost +local:docker

# 3. Перезапустите
docker-compose restart baggage_app
```

### Ошибка подключения к БД

**Проблема**: "Не удалось подключиться к PostgreSQL"

**Решение**:
```bash
# 1. Проверьте статус PostgreSQL
docker-compose ps postgres

# 2. Проверьте логи
docker-compose logs postgres

# 3. Подождите инициализации (~10 секунд)
```

### Порт 5432 занят

**Проблема**: "Port 5432 is already allocated"

**Решение**:
```bash
# Вариант 1: Остановите локальный PostgreSQL
sudo systemctl stop postgresql

# Вариант 2: Измените порт в docker-compose.yml
ports:
  - "5433:5432"  # Вместо 5432:5432
```

## Windows (WSL2)

Для запуска на Windows через WSL2:

```powershell
# В PowerShell: Установите VcXsrv или Xming
# Затем в WSL2:

export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
xhost +
docker-compose up --build
```

## macOS

Требуется XQuartz:

```bash
# 1. Установите XQuartz
brew install --cask xquartz

# 2. Запустите XQuartz и разрешите сетевые подключения
# В настройках XQuartz: Security → Allow connections from network clients

# 3. Перезагрузите Mac

# 4. Разрешите подключения
xhost +localhost

# 5. Запустите приложение
docker-compose up --build
```

## Производственное развертывание

Для production использования:

1. **Измените пароли БД**:
   ```yaml
   environment:
     POSTGRES_PASSWORD: your_secure_password_here
   ```

2. **Используйте Docker secrets**:
   ```yaml
   secrets:
     db_password:
       file: ./secrets/db_password.txt
   ```

3. **Настройте SSL для PostgreSQL**

4. **Используйте reverse proxy** (nginx) для доступа

## Файлы конфигурации

- `Dockerfile` - Образ приложения
- `docker-compose.yml` - Оркестрация сервисов
- `init-db.sql` - Инициализация БД
- `.dockerignore` - Исключения при сборке

## Версии

- **Docker образ**: Ubuntu 22.04
- **Qt**: 6.x
- **PostgreSQL**: 15
- **C++ Standard**: C++17

---

**Автор**: Курсовая работа тема №96
**Версия**: 2.0
