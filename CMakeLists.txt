cmake_minimum_required(VERSION 3.10)
project(ConsoleX)

# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# [핵심] 실행 파일 출력 경로를 프로젝트 루트(CMakeLists.txt가 있는 곳)로 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Thread 라이브러리 찾기 (Linux 환경 필수)
find_package(Threads REQUIRED)

# ==============================================================================
# 1. ConsoleX Core Library (cx_core)
#    - src 폴더의 공통 소스들을 정적 라이브러리로 빌드
# ==============================================================================
add_library(cx_core STATIC
    src/cx_color.cpp
    src/cx_device.cpp
    src/cx_screen.cpp
    src/cx_util.cpp
    src/cx_buffer.cpp
)

# 헤더 파일 경로 포함 (모든 타겟이 include 폴더를 참조하게 함)
target_include_directories(cx_core PUBLIC include)

# ==============================================================================
# 2. Example Applications
#    - example 폴더의 소스들을 개별 실행 파일로 빌드
# ==============================================================================

# App 1: Item Inventory App (ItemApp)
add_executable(ItemApp example/main_item_app.cpp)
target_link_libraries(ItemApp PRIVATE cx_core Threads::Threads)

# App 2: Drawing App (DrawApp)
add_executable(DrawApp example/main_draw_app.cpp)
target_link_libraries(DrawApp PRIVATE cx_core Threads::Threads)

# App 3: Buffer Test (Flicker Test)
add_executable(BufferTest example/main_buffer_test.cpp)
target_link_libraries(BufferTest PRIVATE cx_core Threads::Threads)