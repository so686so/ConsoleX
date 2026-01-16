cmake_minimum_required(VERSION 3.10)
project(ConsoleX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)
find_package(Threads REQUIRED)

set(SOURCES
    src/cx_color.cpp
    src/cx_screen.cpp
    src/cx_device.cpp
    src/cx_util.cpp
    src/main.cpp
)

add_executable(ConsoleX ${SOURCES})
target_link_libraries(ConsoleX PRIVATE Threads::Threads)