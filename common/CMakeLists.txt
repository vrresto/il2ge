set(build_job_id $ENV{CI_JOB_ID})
set(commit_sha $ENV{CI_COMMIT_SHA})

if(NOT build_job_id)
  set(build_job_id "local")
endif()

if(NOT commit_sha)
  set(commit_sha "unknown")
endif()

configure_file(version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

add_definitions(-DINI_ALLOW_MULTILINE=0 -DINI_ALLOW_INLINE_COMMENTS=1)
add_definitions("-DINI_INLINE_COMMENT_PREFIXES=\"\;/\"")

set(CXX_SRCS
  water_map.cpp
  map_loader.cpp
  imf.cpp
  forest.cpp
  map_generator.cpp
  ${PROJECT_SOURCE_DIR}/_modules/inih/cpp/INIReader.cpp
  ${PROJECT_SOURCE_DIR}/_modules/inih/ini.c
  ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

add_library(common ${CXX_SRCS})

target_link_libraries(common render_util)
