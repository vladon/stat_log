cmake_minimum_required(VERSION 2.8)
project("RcStatLog")

include_directories(include)
set(CMAKE_CXX_FLAGS "-std=c++1y")

find_package(Boost 1.54)

add_executable(test.exe test/test.cpp)
