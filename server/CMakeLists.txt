cmake_minimum_required(VERSION 2.6)
project(server)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(SOURCE_FILES src/main.cpp src/server.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
add_executable(server ${SOURCE_FILES})
target_link_libraries(server ${CMAKE_CURRENT_SOURCE_DIR}/../lib/bin/libsocket.a)
