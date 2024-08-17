cmake_minimum_required(VERSION 3.25.3 FATAL_ERROR)

set(CMAKE_SYSTEM_NAME "Windows")
set(CMAKE_C_COMPILER "/usr/bin/i686-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/i686-w64-mingw32-g++")

option(BUILD_EXAMPLES OFF)

add_subdirectory(Extern/wine-discord-rpc)

add_link_options(-static -static-libgcc -static-libstdc++)

project(aos.dll)

add_library(aos SHARED)

# lets disable everything by now, maybe later we will get into it
set(FT_DISABLE_ZLIB ON CACHE BOOL "Disable zlib")
set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable bzip2")
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable brotli")
set(FT_DISABLE_PNG ON CACHE BOOL "Disable png")
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable harfbuzz")
add_subdirectory(Extern/freetype)
add_subdirectory(Extern/json-c)

add_compile_options(-Wall -Wextra -Werror -Wno-error=unused-parameter -std=gnu99)
add_subdirectory(Source)

target_link_libraries(aos PRIVATE json-c-static discord-rpc winhttp freetype)
target_compile_options(json-c PUBLIC "-DBUILD_SHARED_LIBS=OFF")
target_compile_options(discord-rpc PUBLIC "-DBUILD_SHARED_LIBS=OFF")

target_include_directories(aos PUBLIC ${PROJECT_SOURCE_DIR}/Extern/enet/include ${PROJECT_SOURCE_DIR}/Extern/json-c ${CMAKE_SOURCE_DIR}/Extern/wine-discord-rpc/include ${PROJECT_SOURCE_DIR}/Extern/freetype/include)
