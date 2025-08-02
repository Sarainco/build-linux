cmake_minimum_required(VERSION 3.15)
include_guard(GLOBAL)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER %CROSS_COMPILE%gcc)
set(CMAKE_CXX_COMPILER %CROSS_COMPILE%g++)

set(CMAKE_SYSROOT %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot)
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath-link=%TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/lib/ -L %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/lib/ -Wl,--whole-archive -Wl,--no-whole-archive")

set(Libdrm_INCLUDE_DIR %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/include)
set(Libdrm_LIBRARY %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/lib/libdrm.so)

set(EGL_INCLUDE_DIR %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/include)
set(EGL_LIBRARY %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/lib/libmali.so)

set(GLESv2_INCLUDE_DIR %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/include)
set(GLESv2_LIBRARY %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/lib/libmali.so)

set(gbm_INCLUDE_DIR %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/include)
set(gbm_LIBRARY %TOOLCHAIN_DIR%/aarch64-buildroot-linux-gnu/sysroot/usr/lib/libmali.so)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
