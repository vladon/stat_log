cmake_minimum_required(VERSION 2.8)
project("StatLog")

include_directories(include)
set(CMAKE_CXX_FLAGS "-std=c++1y")

find_package(Boost 1.54 COMPONENTS program_options REQUIRED )

#Need to increase the maximum FUSION template size
#See:
# http://stackoverflow.com/questions/26355688/how-do-i-extend-the-fusion-container-size-limits-beyond-50
#for details.
#TODO: Make this a configurable parameter
add_definitions(
    -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
    -DFUSION_MAX_VECTOR_SIZE=30
    -DBOOST_MPL_LIMIT_VECTOR_SIZE=30
    )


set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(test)
add_subdirectory(src)
