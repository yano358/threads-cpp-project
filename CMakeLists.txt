cmake_minimum_required(VERSION 3.10)

project(SO2)

set(CMAKE_BUILD_TYPE Debug)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})  

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories(include)

add_executable(main.out src/main.cpp src/client.cpp)
target_link_libraries(main.out ${CURSES_LIBRARIES} Threads::Threads)

target_compile_options(main.out PRIVATE -g)