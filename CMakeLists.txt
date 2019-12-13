cmake_minimum_required(VERSION 3.0)

if(platform STREQUAL "mingw")
  if(mingw_crash_handler_dir)
    add_subdirectory(${mingw_crash_handler_dir} ${CMAKE_BINARY_DIR}/mingw_crash_handler)
  else()
    add_subdirectory(_modules/mingw_crash_handler)
  endif()
endif(platform STREQUAL "mingw")

set(il2ge_data_dir "il2ge")
set(il2ge_lib_dir "${il2ge_data_dir}/lib")
set(il2ge_cache_dir "${il2ge_data_dir}/cache")
set(il2ge_documentation_dir "${il2ge_data_dir}/documentation")
set(il2ge_thirdparty_dir "${il2ge_data_dir}/documentation/thirdparty")
set(renderutil_base_dir ${il2ge_data_dir})
set(renderutil_shader_dir ${il2ge_data_dir}/shaders)
set(renderutil_cache_dir ${il2ge_cache_dir})
set(render_util_enabled_gl_procs_file ${CMAKE_CURRENT_SOURCE_DIR}/enabled_gl_procs)

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
  set(COMPILER_PREFIX "i686-w64-mingw32")
  set(CMAKE_EXECUTABLE_SUFFIX_CXX ".exe")
  set(CMAKE_SHARED_LIBRARY_PREFIX_C "")
  set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
  set(CMAKE_SHARED_LIBRARY_SUFFIX_C ".dll")
  set(CMAKE_SHARED_LIBRARY_SUFFIX_CXX ".dll")
endif(platform STREQUAL "mingw")


project(il2ge)


set(build_job_id $ENV{CI_JOB_ID})
set(commit_sha $ENV{CI_COMMIT_SHA})

if(NOT build_job_id)
  set(build_job_id "local")
endif()

if(NOT commit_sha)
  set(commit_sha "unknown")
endif()

if(enable_debug)
  set(is_debug_build true)
else()
  set(is_debug_build false)
endif()

if (enable_map_viewer)
  set(render_util_build_viewer true)
else()
  set(render_util_build_viewer false)
endif()

add_definitions(-DIL2GE_DATA_DIR="${il2ge_data_dir}")
add_definitions(-DIL2GE_LIB_DIR="${il2ge_lib_dir}")
add_definitions(-DIL2GE_CACHE_DIR="${il2ge_cache_dir}")
if (enable_debug)
  add_definitions(-DRENDER_UTIL_ENABLE_DEBUG=1)
endif()

if(platform_wine)
  set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
endif(platform_wine)

if(platform_mingw)
  set(CMAKE_C_FLAGS "-static-libgcc -static-libstdc++ -mthreads")
  set(CMAKE_CXX_FLAGS "-static-libgcc -static-libstdc++ -mthreads")
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
  add_definitions(-DMINGW_HAS_SECURE_API)
  add_definitions(-DIL2GE_USE_MSVCRT)
  add_definitions(-DRENDER_UTIL_USE_MSVCRT)
  include_directories(${mingw_crash_handler_SOURCE_DIR})
endif(platform_mingw)

if(NOT renderutil_dir)
  set(renderutil_dir ${PROJECT_SOURCE_DIR}/_modules/render_util)
endif(NOT renderutil_dir)

add_subdirectory(${renderutil_dir} ${CMAKE_BINARY_DIR}/render_util)

include(${renderutil_dir}/cflags.cmake)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}
  ${renderutil_dir}/include
  ${render_util_BINARY_DIR}/include
  ${render_util_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/_modules/inih/cpp
)


ExternalProject_Get_Property(glm SOURCE_DIR)
include_directories(${SOURCE_DIR})


ExternalProject_Get_Property(half SOURCE_DIR)
include_directories(${SOURCE_DIR}/include)


add_subdirectory(common)
if(enable_map_viewer)
  add_subdirectory(map_viewer)
  add_subdirectory(map_editor)
endif()
if(platform_mingw AND NOT disable_il2ge)
  add_subdirectory(core_wrapper)
endif()


install(DIRECTORY object_shaders
  DESTINATION ${il2ge_data_dir}
)

install(DIRECTORY shaders
  DESTINATION ${il2ge_data_dir}
)

install(FILES resources/dummy.tga
  DESTINATION "#SAS/il2ge/il2ge"
)

if(platform_mingw)
  install(PROGRAMS ${mingw_crash_handler_BINARY_DIR}/mingw_crash_handler.dll
    DESTINATION ${il2ge_lib_dir}
  )
endif(platform_mingw)


install(FILES README.md
  DESTINATION ${il2ge_documentation_dir}
)

install(FILES _modules/mingw_crash_handler/LICENSE.txt
  DESTINATION ${il2ge_thirdparty_dir}/mingw_crash_handler
#   RENAME mingw_crash_handler_LICENSE.txt
)

install(FILES _modules/inih/LICENSE.txt
  DESTINATION ${il2ge_thirdparty_dir}/inih
#   RENAME inih_LICENSE.txt
)

install(FILES _modules/inih/README.md
  DESTINATION ${il2ge_thirdparty_dir}/inih
#   RENAME inih_README.md
)


add_custom_target(run
  WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
  COMMAND wine il2fb.exe
)

add_custom_target(run_viewer
  WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
  COMMAND wine map_viewer.exe
)

add_custom_target(refresh_enabled_gl_procs
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND bash
    ${renderutil_dir}/scripts/print-used-gl-procs.bash
    ${PROJECT_SOURCE_DIR} ${renderutil_dir}
      > ${render_util_enabled_gl_procs_file}
)

add_custom_target(installer
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND BUILD=${build_job_id} SRC_DIR=${CMAKE_INSTALL_PREFIX} DEST_DIR=${CMAKE_CURRENT_BINARY_DIR} makensis ${PROJECT_SOURCE_DIR}/il2ge.nsi
)
