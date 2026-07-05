cmake_minimum_required(VERSION 3.20)

project(drone_simulation CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS # for IDE
		include/*.hpp
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
		src/*.cpp
)

add_executable(drone_simulation ${SOURCES} ${HEADERS})

target_include_directories(drone_simulation PUBLIC
		include/
		include/core
		include/core/config
		include/core/providers
		include/core/solvers
		include/core/states
		include/interfaces
		include/lib
)

target_compile_options(drone_simulation PRIVATE -Wall -Wextra -pedantic)

