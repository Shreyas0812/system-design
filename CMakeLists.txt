cmake_minimum_required(VERSION 3.10)
project(system_design CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(circular_buffer_blocking CircularBuffer_ThreadBlocking.cpp)
target_link_libraries(circular_buffer_blocking PRIVATE Threads::Threads)

add_executable(circular_buffer_nonblocking CircularBuffer_ThreadNonBlocking.cpp)
target_link_libraries(circular_buffer_nonblocking PRIVATE Threads::Threads)

add_executable(thread_pool ThreadPool.cpp)
target_link_libraries(thread_pool PRIVATE Threads::Threads)
