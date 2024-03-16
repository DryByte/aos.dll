cmake_minimum_required(VERSION 3.25.3 FATAL_ERROR)

set(CMAKE_SYSTEM_NAME "Windows")
set(CMAKE_C_COMPILER "/usr/bin/i686-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/i686-w64-mingw32-g++")

option(BUILD_EXAMPLES OFF)

add_subdirectory(Extern/wine-discord-rpc)
#add_subdirectory(Extern/json-c)

add_compile_options(-Wall -Wextra -Werror -Wno-error=unused-parameter -std=gnu99)
add_link_options(-static -static-libgcc -static-libstdc++)

project(aos.dll)

add_library(aos SHARED)
add_subdirectory(Source)
target_link_libraries(aos discord-rpc)
target_include_directories(aos PUBLIC ${PROJECT_SOURCE_DIR}/Extern/enet/include ${CMAKE_SOURCE_DIR}/Extern/wine-discord-rpc/include)
#target_link_libraries(aos json-c)