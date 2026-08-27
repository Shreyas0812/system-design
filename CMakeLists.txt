cmake_minimum_required(VERSION 3.10)
project(system_design CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(circular_buffer CircularBuffer.cpp)
target_link_libraries(circular_buffer PRIVATE Threads::Threads)
