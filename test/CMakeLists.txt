macro(stat_log_link_libs test_name)
    TARGET_LINK_LIBRARIES (${test_name}_generator ${test_name} stat_log ${Boost_LIBRARIES})
    TARGET_LINK_LIBRARIES (${test_name}_controller ${test_name} stat_log ${Boost_LIBRARIES})
    if(UNIX AND NOT APPLE)
        #librt is needed for boost shared_memory on linux
        # but not on apple, apparently.
        TARGET_LINK_LIBRARIES (${test_name}_generator rt -pthread)
        TARGET_LINK_LIBRARIES (${test_name}_controller rt -pthread)
    endif()
endmacro()

include_directories(include)
add_subdirectory(test_basic)
add_subdirectory(test_full)
