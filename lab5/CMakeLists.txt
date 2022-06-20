cmake_minimum_required(VERSION 3.22)
project(lab_5)

set(CMAKE_CXX_STANDARD 98)

add_executable(lab_5 server.cpp employee.cpp defs.h client.cpp)
