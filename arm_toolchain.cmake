if(NOT DEFINED ENV{SYSROOT_BASE})
    message(FATAL_ERROR "SYSROOT_BASE is not defined!")
    # set (SYSROOT_BASE "/rc/cce_bsp-v0.7.1/sysroot-modem/")
endif()
set (SYSROOT_BASE "$ENV{SYSROOT_BASE}/")
message("SYSROOT_BASE= ${SYSROOT_BASE}")
set (SYSROOT_ARM ${SYSROOT_BASE}/arm-buildroot-linux-gnueabihf/sysroot)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l)

set(CMAKE_FIND_ROOT_PATH "${SYSROOT_ARM}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${SYSROOT_ARM}")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} --sysroot=${SYSROOT_ARM}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_CXX_FLAGS} -L${SYSROOT_ARM}/lib -L ${SYSROOT_ARM}/usr/lib")
set(CMAKE_C_COMPILER "${SYSROOT_BASE}/../opt/ext-toolchain/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${SYSROOT_BASE}/../opt/ext-toolchain/bin/arm-linux-gnueabihf-g++")

