cmake_minimum_required(VERSION 2.8)
project("RcStatLog")

include_directories(include)
set(CMAKE_CXX_FLAGS "-std=c++1y")

find_package(Boost 1.54 COMPONENTS program_options REQUIRED )

# add_executable(test.exe test/test.cpp)
add_executable(RcStatLog test/test.cpp)

TARGET_LINK_LIBRARIES (RcStatLog ${Boost_LIBRARIES})
