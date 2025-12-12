cmake_minimum_required(VERSION 3.16)
project(BaggageSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Поиск Qt6 (или Qt5 если Qt6 недоступен)
find_package(Qt6 COMPONENTS Core Widgets Sql QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets Sql)
endif()

# Включение директорий
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/BaggageRecord.cpp
    src/BaggageManager.cpp
    src/DatabaseManager.cpp
    src/MainWindow.cpp
    src/AddRecordDialog.cpp
    src/FilterDialog.cpp
    src/DeleteByFlightDialog.cpp
    src/ChangeItemsDialog.cpp
)

# Заголовочные файлы
set(HEADERS
    include/BaggageRecord.h
    include/BaggageManager.h
    include/DatabaseManager.h
    include/MainWindow.h
    include/AddRecordDialog.h
    include/FilterDialog.h
    include/DeleteByFlightDialog.h
    include/ChangeItemsDialog.h
)

# Ресурсные файлы
set(RESOURCES
    resources/resources.qrc
)

# Создание исполняемого файла
if(Qt6_FOUND)
    qt_add_executable(BaggageSystem ${SOURCES} ${HEADERS} ${RESOURCES})
    target_link_libraries(BaggageSystem PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
else()
    add_executable(BaggageSystem ${SOURCES} ${HEADERS})
    qt5_add_resources(RESOURCES_OUT ${RESOURCES})
    target_sources(BaggageSystem PRIVATE ${RESOURCES_OUT})
    target_link_libraries(BaggageSystem PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql)
endif()

# Установка свойств для Windows
if(WIN32)
    set_target_properties(BaggageSystem PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
