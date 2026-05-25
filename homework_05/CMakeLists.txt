cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(telemetry CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# telemetry - бібліотека з логікою читання, парсингу і підсумку;
add_library(telemetry src/telemetry.cpp)

# include-директорія підключена через target_include_directories.
target_include_directories(telemetry PUBLIC include/)

# telemetry_check - виконуваний файл, який читає шлях до файла з argv[1];
add_executable(telemetry_check src/main.cpp)

# telemetry_check лінкується до telemetry;
target_link_libraries(telemetry_check PRIVATE telemetry)

# Базові compiler warnings допомагають побачити підозрілі місця до runtime.
target_compile_options(telemetry PRIVATE -Wall -Wextra -pedantic)
