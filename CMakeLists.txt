cmake_minimum_required(VERSION 3.5)

project(zookeeper_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS -DTHREADED)

add_executable(zookeeper_queue main.cpp)

target_link_libraries(zookeeper_queue zookeeper_mt)