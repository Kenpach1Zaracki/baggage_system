# Инструкции по тестированию курсовой работы

## Шаг 1: Проверка всех файлов

Убедитесь, что присутствуют следующие файлы:

```bash
tree -L 2 -I 'build|.git'
```

Должны быть:
```
├── CMakeLists.txt
├── Dockerfile
├── docker-compose.yml
├── init-db.sql
├── .dockerignore
├── include/
│   ├── DatabaseManager.h          # ★ НОВОЕ
│   ├── BaggageManager.h
│   ├── BaggageRecord.h
│   └── (диалоги...)
├── src/
│   ├── DatabaseManager.cpp        # ★ НОВОЕ
│   ├── main.cpp                   # ★ ОБНОВЛЕНО (БД + стили)
│   ├── BaggageManager.cpp         # ★ ОБНОВЛЕНО (PostgreSQL)
│   └── (остальные файлы...)
├── resources/                     # ★ НОВОЕ
│   ├── styles.qss                 # ★ QSS дизайн
│   └── resources.qrc
├── README.md                      # ★ ОБНОВЛЕНО
├── DOCUMENTATION.md
├── DOCKER.md                      # ★ НОВОЕ
├── SUMMARY.md                     # ★ НОВОЕ
└── TEST_INSTRUCTIONS.md           # этот файл
```

---

## Шаг 2: Тестирование Docker сборки

### 2.1 Сборка образа приложения

```bash
docker build -t baggage_system:test .
```

**Ожидаемый результат**: Успешная сборка без ошибок

**Что проверяется**:
- ✅ CMakeLists.txt корректен
- ✅ Все .h и .cpp файлы компилируются
- ✅ Qt SQL модуль линкуется
- ✅ Ресурсы (QSS) подключаются

### 2.2 Проверка размера образа

```bash
docker images baggage_system:test
```

**Ожидаемый размер**: ~150-250 MB (благодаря multi-stage build)

---

## Шаг 3: Запуск PostgreSQL

```bash
# Только БД (без GUI)
docker-compose up postgres -d
```

**Проверка**:
```bash
# Проверить логи
docker-compose logs postgres

# Должно быть:
# ✅ "database system is ready to accept connections"
# ✅ "База данных багажной системы инициализирована успешно"
# ✅ "Создано записей: 5"
```

**Подключение к БД**:
```bash
docker-compose exec postgres psql -U postgres -d baggage_db -c "SELECT COUNT(*) FROM baggage_records;"
```

**Ожидаемый вывод**: 5 записей

---

## Шаг 4: Запуск приложения

### Linux

```bash
# 1. Разрешить X11
xhost +local:docker

# 2. Запустить всё
docker-compose up

# В новом терминале проверить статус
docker-compose ps
```

### macOS

```bash
# 1. Установить XQuartz (если нет)
brew install --cask xquartz

# 2. Запустить XQuartz, в настройках включить "Allow connections from network clients"

# 3. Перезагрузить Mac

# 4. Разрешить X11
xhost +localhost

# 5. Запустить
docker-compose up
```

### Windows (WSL2)

```powershell
# В PowerShell: Установить VcXsrv
# Запустить VcXsrv с опцией "Disable access control"

# В WSL2:
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
docker-compose up
```

---

## Шаг 5: Проверка функциональности

Когда приложение запустится, проверьте:

### 5.1 Главное окно

- ✅ Окно открылось
- ✅ **Дизайн применен** (синие градиенты, не серые кнопки!)
- ✅ Таблица показывает 5 тестовых записей
- ✅ Меню "Файл", "Операции", "Помощь"
- ✅ Панель инструментов с кнопками

### 5.2 Функция 1: Создать файл

```
Нажать "1. Создать файл"
→ Диалог сохранения (можно отменить, т.к. БД уже создана)
→ Статус бар: "База данных очищена"
```

### 5.3 Функция 6: Добавить запись

```
Нажать "6. Добавить запись"
→ Диалог с полями:
   - № рейса: "TEST123"
   - ФИО: "Тестов Тест Тестович"
   - Кол-во вещей: 2
   - Вес 1: 10.5
   - Вес 2: 15.0
→ "OK"
→ Запись добавилась в таблицу
```

### 5.4 Функция 3: Фильтр

```
Нажать "3. Фильтр (1 вещь, 20-30 кг)"
→ Окно с 2 записями:
   - AF567 - Петров П.П. (25.00 кг)
   - LH456 - Козлов А.В. (23.50 кг)
```

### 5.5 Функция 4-5: Файл сводки

```
Нажать "4. Создать файл сводки"
→ Сохранить как "summary.txt"
→ Нажать "5. Показать файл сводки"
→ Открывается в текстовом редакторе
→ Формат:
   № рейса    Ф.И.О.          Общий вес (кг)
   ========================================
   SU1234     Иванов И.И.     55.80
   ...
```

### 5.6 Функция 7: Удалить по рейсам

```
Нажать "7. Удалить по рейсам"
→ Ввести:
   TEST123
   AF567
→ "OK"
→ Удалено 2 записи
→ Таблица обновилась
```

### 5.7 Функция 8: Изменить количество вещей

```
Нажать "8. Изменить кол-во вещей"
→ ФИО: "Иванов Иван Иванович"
→ Новое кол-во: 1
→ Вес 1: 50.0
→ "OK"
→ Запись обновилась в таблице
```

---

## Шаг 6: Проверка БД

В другом терминале:

```bash
# Подключиться к PostgreSQL
docker-compose exec postgres psql -U postgres -d baggage_db

# SQL команды:
\dt                                              # Список таблиц
SELECT * FROM baggage_records;                   # Все записи
SELECT COUNT(*) FROM baggage_records;            # Количество
\d baggage_records                               # Схема таблицы
\di                                              # Индексы
\q                                               # Выход
```

**Ожидается**:
- ✅ Таблица `baggage_records` существует
- ✅ 2 индекса: `idx_flight_number`, `idx_passenger_name`
- ✅ Записи соответствуют таблице в GUI

---

## Шаг 7: Проверка дизайна

### 7.1 Визуальные элементы

- ✅ **Кнопки**: Градиент от синего (#667eea) к фиолетовому (#764ba2)
- ✅ **Hover эффект**: Кнопки становятся светлее при наведении
- ✅ **Таблица**: Чередующиеся строки (белый / #f8f9ff)
- ✅ **Заголовки таблицы**: Белый текст на градиентном фоне
- ✅ **Меню**: Плавные переходы при наведении
- ✅ **Скроллбары**: Кастомный дизайн с градиентом

### 7.2 Сравнение

**БЫЛО** (без QSS):
- Серые кнопки
- Простая таблица
- Стандартное меню Qt

**СТАЛО** (с QSS):
- Градиентные кнопки
- Стильная таблица с чередованием
- Современное меню с эффектами

---

## Шаг 8: Остановка и очистка

```bash
# Остановить контейнеры
docker-compose down

# Очистка (удалит данные БД!)
docker-compose down -v

# Удалить образ
docker rmi baggage_system_baggage_app
```

---

## Контрольный список (Checklist)

### Код

- [x] DatabaseManager.h/cpp - работа с PostgreSQL
- [x] BaggageManager.cpp - обновлен для PostgreSQL
- [x] main.cpp - подключение к БД и загрузка стилей
- [x] CMakeLists.txt - Qt SQL модуль + ресурсы
- [x] resources/styles.qss - QSS стили

### Docker

- [x] Dockerfile - multi-stage build
- [x] docker-compose.yml - PostgreSQL + App
- [x] init-db.sql - схема и тестовые данные
- [x] .dockerignore - оптимизация сборки

### Документация

- [x] README.md - обновлен (PostgreSQL + Docker + UI)
- [x] DOCKER.md - инструкции по Docker
- [x] SUMMARY.md - итоговая сводка для преподавателя
- [x] TEST_INSTRUCTIONS.md - этот файл

### Функциональность (8 функций)

- [x] 1. Создать файл (очистить БД)
- [x] 2. Показать содержимое (SELECT *)
- [x] 3. Фильтр 1 вещь 20-30кг
- [x] 4. Создать файл сводки
- [x] 5. Показать файл сводки
- [x] 6. Добавить запись (INSERT)
- [x] 7. Удалить по рейсам (DELETE)
- [x] 8. Изменить кол-во вещей (UPDATE)

### ТЗ (п.4.5)

- [x] СУБД: PostgreSQL ✅
- [x] Docker: Dockerfile + docker-compose.yml ✅

### Дизайн

- [x] QSS стили применены
- [x] Градиенты работают
- [x] Hover эффекты
- [x] Русский язык

---

## Возможные проблемы

### 1. "Qt module not found: Sql"

**Решение**: Установите `libqt6sql6-psql` или `qt6-qtbase-postgresql`

### 2. GUI не отображается

**Решение**:
```bash
xhost +local:docker
echo $DISPLAY  # Должно быть ":0" или ":1"
```

### 3. PostgreSQL не запускается

**Решение**:
```bash
# Проверить логи
docker-compose logs postgres

# Остановить локальный PostgreSQL если конфликт портов
sudo systemctl stop postgresql
```

### 4. Стили не применяются

**Проверка**:
```bash
# Убедитесь что файл ресурсов есть
cat resources/resources.qrc

# Пересоберите
rm -rf build/*
cmake ..
cmake --build .
```

---

## Для защиты курсовой

### Что показать

1. **Запуск через Docker**:
   ```bash
   docker-compose up --build
   ```

2. **Дизайн**: Показать градиенты, hover эффекты

3. **Все 8 функций**: Выполнить каждую функцию по порядку

4. **PostgreSQL**: Открыть DBeaver/pgAdmin и показать таблицу

5. **Код**: Показать DatabaseManager.cpp и styles.qss

### Что рассказать

- **PostgreSQL**: ACID, индексы, транзакции, масштабируемость до 10,000+ записей
- **Docker**: Портативность, изоляция, легкое развертывание на любой машине
- **Qt QSS**: Кастомизация UI, современный дизайн, Material Design
- **Архитектура**: Separation of Concerns (Database Layer → Manager → View)

---

**Проект полностью готов к защите!** ✅
