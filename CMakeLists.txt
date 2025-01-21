cmake_minimum_required(VERSION 3.22)

project(bagheera)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    src/main.cpp
    src/events.cpp
    src/utils.cpp
)

set (CMAKE_EXPORT_COMPILE_COMMANDS on)
include_directories(include)
add_executable(main ${SOURCES})

target_link_libraries(main xcb)
target_link_libraries(main xcb-composite)
