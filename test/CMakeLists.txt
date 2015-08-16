add_executable(SimpleTest test.cpp)

TARGET_LINK_LIBRARIES (SimpleTest ${Boost_LIBRARIES} stat_log)
if(UNIX AND NOT APPLE)
    #librt is needed for boost shared_memory on linux
    # but not on apple, apparently.
    TARGET_LINK_LIBRARIES (SimpleTest rt)
endif()
