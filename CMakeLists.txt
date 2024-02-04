cmake_minimum_required(VERSION 3.25.3 FATAL_ERROR)

set(CMAKE_SYSTEM_NAME "Windows")
set(CMAKE_C_COMPILER "/usr/bin/i686-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/i686-w64-mingw32-g++")

add_compile_options(-Wall -Wextra -Werror -Wno-error=unused-parameter -std=gnu99)

project(aos.dll)

add_library(aos SHARED)

add_subdirectory(Source)
target_include_directories(aos PUBLIC ${PROJECT_SOURCE_DIR}/Extern/enet/include)