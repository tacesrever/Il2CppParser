
project(parser CXX)
set(toolchain your_ndk_toolchain_path)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSROOT ${toolchain}/sysroot)

set(CMAKE_CXX_COMPILER ${toolchain}/bin/arm-linux-androideabi-g++)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include_directories(include)
include_directories(libil2cpp)

add_library(parser SHARED src/parser.cpp)

