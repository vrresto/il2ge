cmake_minimum_required(VERSION 3.0)
project(il2ge)

set(renderutil_dir ${PROJECT_SOURCE_DIR}/render_util)

add_subdirectory(${renderutil_dir} ${CMAKE_BINARY_DIR}/render_util)

include(${renderutil_dir}/cflags.cmake)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${renderutil_dir}/include)
include_directories(${render_util_BINARY_DIR})

add_subdirectory(common)
add_subdirectory(map_viewer)
add_subdirectory(core_wrapper)
