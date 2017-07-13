cmake_minimum_required(VERSION 2.6)
project(client)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/main.cpp src/socket.h src/socket.cpp src/client.cpp)
add_executable(client ${SOURCE_FILES})