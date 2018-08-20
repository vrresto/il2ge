cmake_minimum_required(VERSION 3.0)

if(platform STREQUAL "mingw")
  add_subdirectory(mingw_crash_handler)
endif(platform STREQUAL "mingw")

set(core_wrapper_library_name "il2_core_wrapper")
set(il2ge_data_dir "il2ge")
set (renderutil_data_dir ${il2ge_data_dir})

if(platform STREQUAL "wine")
  set(platform_wine 1)
  set(CMAKE_C_COMPILER_WORKS 1)
  set(CMAKE_CXX_COMPILER_WORKS 1)
  set(CMAKE_C_COMPILER "winegcc")
  set(CMAKE_CXX_COMPILER "wineg++")
  set(CMAKE_EXECUTABLE_SUFFIX_CXX ".exe")
  set(CMAKE_SHARED_LIBRARY_PREFIX_C "")
  set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
  set(CMAKE_SHARED_LIBRARY_SUFFIX_C ".dll.so")
  set(CMAKE_SHARED_LIBRARY_SUFFIX_CXX ".dll.so")
endif(platform STREQUAL "wine")

if(platform STREQUAL "mingw")
  set(platform_mingw 1)
  set(no_std_thread 1)
  set(CMAKE_C_COMPILER_WORKS 1)
  set(CMAKE_CXX_COMPILER_WORKS 1)
  set(CMAKE_C_COMPILER "i686-w64-mingw32-gcc")
  set(CMAKE_CXX_COMPILER "i686-w64-mingw32-g++")
  set(CMAKE_AR "i686-w64-mingw32-ar")
  set(CMAKE_EXECUTABLE_SUFFIX_CXX ".exe")
  set(CMAKE_SHARED_LIBRARY_PREFIX_C "")
  set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
  set(CMAKE_SHARED_LIBRARY_SUFFIX_C ".dll")
  set(CMAKE_SHARED_LIBRARY_SUFFIX_CXX ".dll")
endif(platform STREQUAL "mingw")


project(il2ge)

add_definitions(-DIL2GE_DATA_DIR="${il2ge_data_dir}")
if(static_core_wrapper)
  add_definitions(-DSTATIC_CORE_WRAPPER)
endif(static_core_wrapper)

if(platform_wine)
  set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
endif(platform_wine)

if(platform_mingw)
  set(CMAKE_C_FLAGS "-static-libgcc -static-libstdc++")
  set(CMAKE_CXX_FLAGS "-static-libgcc -static-libstdc++")
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
  add_definitions(-DMINGW_HAS_SECURE_API)
endif(platform_mingw)

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

if(platform_mingw)
  ExternalProject_Get_Property(glm SOURCE_DIR)
  include_directories(${SOURCE_DIR})
endif(platform_mingw)

add_subdirectory(common)
add_subdirectory(core_wrapper)
add_subdirectory(loader)
if(NOT platform_mingw)
  add_subdirectory(map_viewer)
endif(NOT platform_mingw)

install(DIRECTORY ${renderutil_dir}/shaders
  DESTINATION ${il2ge_data_dir}
)

if(platform_mingw)
  install(PROGRAMS ${mingw_crash_handler_BINARY_DIR}/mingw_crash_handler.dll
    DESTINATION ${il2ge_data_dir}
  )
endif(platform_mingw)
