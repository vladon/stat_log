
add_executable(test test.cpp)

TARGET_LINK_LIBRARIES (test ${Boost_LIBRARIES} stat_log)
