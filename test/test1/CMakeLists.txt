
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

add_library(test1 common.cpp)
add_executable(test1_generator generator.cpp)
add_executable(test1_controller controller.cpp)

TARGET_LINK_LIBRARIES (test1_generator ${Boost_LIBRARIES} stat_log test1)
TARGET_LINK_LIBRARIES (test1_controller ${Boost_LIBRARIES} stat_log test1)
if(UNIX AND NOT APPLE)
    #librt is needed for boost shared_memory on linux
    # but not on apple, apparently.
    TARGET_LINK_LIBRARIES (test1_generator rt)
    TARGET_LINK_LIBRARIES (test1_controller rt)
endif()
