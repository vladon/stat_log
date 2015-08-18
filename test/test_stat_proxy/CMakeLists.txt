
add_library(test_proxy common.cpp mac_gen.cpp sis_gen.cpp hw_intf_gen.cpp)
add_executable(test_proxy_gen generator.cpp)
add_executable(test_proxy_controller controller.cpp)

TARGET_LINK_LIBRARIES (test_proxy_gen ${Boost_LIBRARIES} stat_log test_proxy)
TARGET_LINK_LIBRARIES (test_proxy_controller ${Boost_LIBRARIES} stat_log test_proxy)
if(UNIX AND NOT APPLE)
    #librt is needed for boost shared_memory on linux
    # but not on apple, apparently.
    TARGET_LINK_LIBRARIES (test_proxy_gen rt)
    TARGET_LINK_LIBRARIES (test_proxy_controller rt)
endif()
