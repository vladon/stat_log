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

include_directories(include)
add_subdirectory(test1)
add_subdirectory(test_stat_proxy)
