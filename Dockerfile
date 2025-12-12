# Многоэтапная сборка для минимизации размера образа
FROM ubuntu:22.04 AS builder

# Устанавливаем переменные окружения для избежания интерактивных запросов
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow

# Обновляем пакеты и устанавливаем зависимости для сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6sql6-psql \
    libgl1-mesa-dev \
    libxkbcommon-x11-0 \
    libxcb-xinerama0 \
    libxcb-cursor0 \
    && rm -rf /var/lib/apt/lists/*

# Создаем рабочую директорию
WORKDIR /app

# Копируем исходный код
COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/
COPY resources/ ./resources/

# Создаем директорию для сборки
RUN mkdir build && cd build && \
    cmake .. && \
    cmake --build . --config Release

# ============================================
# Финальный образ
# ============================================
FROM ubuntu:22.04

# Устанавливаем переменные окружения
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow
ENV QT_QPA_PLATFORM=xcb
ENV DISPLAY=:0

# Устанавливаем только runtime зависимости
RUN apt-get update && apt-get install -y \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6sql6 \
    libqt6sql6-psql \
    libgl1-mesa-glx \
    libxkbcommon-x11-0 \
    libxcb-xinerama0 \
    libxcb-cursor0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libfontconfig1 \
    libdbus-1-3 \
    && rm -rf /var/lib/apt/lists/*

# Создаем пользователя для запуска приложения
RUN useradd -m -s /bin/bash appuser

# Создаем рабочую директорию
WORKDIR /app

# Копируем собранное приложение из builder
COPY --from=builder /app/build/BaggageSystem /app/BaggageSystem

# Даем права на выполнение
RUN chmod +x /app/BaggageSystem

# Переключаемся на непривилегированного пользователя
USER appuser

# Метаданные
LABEL maintainer="Курсовая работа - Система багажа"
LABEL description="Qt приложение для управления багажом пассажиров с PostgreSQL"
LABEL version="2.0"

# Запускаем приложение
CMD ["/app/BaggageSystem"]
