macro(stat_log_link_libs test_name)
    TARGET_LINK_LIBRARIES (${test_name}_generator ${test_name} stat_log ${Boost_LIBRARIES})
    TARGET_LINK_LIBRARIES (${test_name}_controller ${test_name} stat_log ${Boost_LIBRARIES})
    if(UNIX AND NOT APPLE)
        #librt is needed for boost shared_memory on Linux
        # but not on apple, apparently.
        #Note the "-lstdc++" is only needed if we want to use the clang compiler
        # (via export CXX=clang) on linux but it doesn't hurt to leave it
        # there if using g++.
        TARGET_LINK_LIBRARIES (${test_name}_generator rt -pthread -lstdc++)
        TARGET_LINK_LIBRARIES (${test_name}_controller rt -pthread -lstdc++)
    endif()
endmacro()

include_directories(include)
add_subdirectory(test_basic)
add_subdirectory(test_full)
