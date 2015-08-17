cmake_minimum_required(VERSION 2.8)
project("StatLog")

include_directories(include)
set(CMAKE_CXX_FLAGS "-std=c++1y")

find_package(Boost 1.54 COMPONENTS program_options REQUIRED )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(test)
add_subdirectory(src)
