cmake_minimum_required(VERSION 3.20)

project(drone_simulation CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(drone_simulation src/main.cpp)

target_include_directories(drone_simulation PUBLIC include)

target_compile_options(drone_simulation PRIVATE -Wall -Wextra -pedantic)

