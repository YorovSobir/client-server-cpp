cmake_minimum_required(VERSION 2.6)
project(client)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/main.cpp src/client.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
add_executable(client ${SOURCE_FILES})
target_link_libraries(client ${CMAKE_CURRENT_SOURCE_DIR}/../lib/bin/libsocket.a)

