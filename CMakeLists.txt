cmake_minimum_required(VERSION 3.22)
project(task2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_executable(task2 src/synch.cpp)

target_link_libraries(task2 PRIVATE Threads::Threads stdc++fs)
