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

macro(stat_log_link_libs test_name)
    TARGET_LINK_LIBRARIES (${test_name}_generator ${Boost_LIBRARIES} stat_log ${test_name})
    TARGET_LINK_LIBRARIES (${test_name}_controller ${Boost_LIBRARIES} stat_log ${test_name})
    if(UNIX AND NOT APPLE)
        #librt is needed for boost shared_memory on linux
        # but not on apple, apparently.
        TARGET_LINK_LIBRARIES (${test_name}_generator rt)
        TARGET_LINK_LIBRARIES (${test_name}_controller rt)
    endif()
endmacro()

include_directories(include)
add_subdirectory(test_multi_gens)
add_subdirectory(test_stat_proxy)
