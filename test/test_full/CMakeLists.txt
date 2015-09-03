get_filename_component(TEST_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_library(${TEST_NAME} common.cpp mac_gen.cpp sis_gen.cpp hw_intf_gen.cpp)
add_executable(${TEST_NAME}_generator generator.cpp)
add_executable(${TEST_NAME}_controller controller.cpp)
stat_log_link_libs(${TEST_NAME})

