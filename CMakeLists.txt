cmake_minimum_required(VERSION 3.25.3 FATAL_ERROR)

set(CMAKE_SYSTEM_NAME "Windows")
set(CMAKE_C_COMPILER "/usr/bin/i686-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/i686-w64-mingw32-g++")

option(BUILD_EXAMPLES OFF)

add_subdirectory(Extern/wine-discord-rpc)

add_compile_options(-Wall -Wextra -Werror -Wno-error=unused-parameter -std=gnu99)
add_link_options(-static -static-libgcc -static-libstdc++)

project(aos.dll)

add_library(aos SHARED)
add_subdirectory(Source)
add_subdirectory(Extern/json-c)

target_link_libraries(aos PRIVATE json-c-static)
target_compile_options(json-c PUBLIC "-DBUILD_SHARED_LIBS=OFF")

target_include_directories(aos PUBLIC ${PROJECT_SOURCE_DIR}/Extern/enet/include ${PROJECT_SOURCE_DIR}/Extern/json-c)
