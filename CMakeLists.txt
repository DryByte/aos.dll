cmake_minimum_required(VERSION 3.26.2 FATAL_ERROR)

set(CMAKE_SYSTEM_NAME "Windows")
set(CMAKE_C_COMPILER "/usr/bin/i686-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/i686-w64-mingw32-g++")

project(aos.dll)

file(GLOB srcFiles "./src/*.c")

add_library(aos SHARED ${srcFiles} main.c)

target_include_directories(aos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(aos ${CMAKE_CURRENT_SOURCE_DIR}/libs/libenet.a ws2_32 winmm)