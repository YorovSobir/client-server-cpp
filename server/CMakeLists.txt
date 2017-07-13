cmake_minimum_required(VERSION 2.6)
project(server)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/main.cpp src/socket.h src/socket.cpp src/server.cpp)
add_executable(server ${SOURCE_FILES})