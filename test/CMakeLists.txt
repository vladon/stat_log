macro(stat_log_link_libs test_name)
    TARGET_LINK_LIBRARIES (${test_name}_generator ${Boost_LIBRARIES} stat_log ${test_name})
    TARGET_LINK_LIBRARIES (${test_name}_controller ${Boost_LIBRARIES} stat_log ${test_name})
    if(UNIX AND NOT APPLE)
        #librt is needed for boost shared_memory on linux
        # but not on apple, apparently.
        TARGET_LINK_LIBRARIES (${test_name}_generator rt -pthread)
        TARGET_LINK_LIBRARIES (${test_name}_controller rt)
    endif()
endmacro()

include_directories(include)
add_subdirectory(test_multi_gens)
add_subdirectory(test_stat_proxy)
