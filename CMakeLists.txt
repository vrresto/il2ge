cmake_minimum_required(VERSION 3.0)

set(core_wrapper_library_name "il2_core_wrapper")
set(il2ge_data_dir "il2ge")
set (renderutil_data_dir ${il2ge_data_dir})

if(platform STREQUAL "wine")
  set(CMAKE_C_COMPILER_WORKS 1)
  set(CMAKE_CXX_COMPILER_WORKS 1)
  set(CMAKE_C_COMPILER "winegcc")
  set(CMAKE_CXX_COMPILER "wineg++")
  set(CMAKE_EXECUTABLE_SUFFIX_CXX ".exe")
  set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
  set(CMAKE_SHARED_LIBRARY_SUFFIX_CXX ".dll.so")
endif(platform STREQUAL "wine")

project(il2ge)

add_definitions(-DIL2GE_DATA_DIR="${il2ge_data_dir}")

if(platform STREQUAL "wine")
  set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
endif(platform STREQUAL "wine")

if(NOT renderutil_dir)
  set(renderutil_dir ${PROJECT_SOURCE_DIR}/render_util)
endif(NOT renderutil_dir)

add_subdirectory(${renderutil_dir} ${CMAKE_BINARY_DIR}/render_util)

include(${renderutil_dir}/cflags.cmake)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}
  ${renderutil_dir}/include
  ${render_util_BINARY_DIR}
)

add_subdirectory(common)
add_subdirectory(map_viewer)
add_subdirectory(core_wrapper)
add_subdirectory(loader)

install(DIRECTORY render_util/shaders
  DESTINATION ${il2ge_data_dir}
)
