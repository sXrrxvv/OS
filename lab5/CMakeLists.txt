cmake_minimum_required(VERSION 3.21)
project(Lab5)

set(CMAKE_CXX_STANDARD 98)

add_executable(server server.cpp employee.h)
add_executable(client client.cpp employee.h)
