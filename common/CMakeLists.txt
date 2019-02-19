configure_file(version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

add_definitions(-DINI_ALLOW_MULTILINE=0 -DINI_ALLOW_INLINE_COMMENTS=1)
add_definitions("-DINI_INLINE_COMMENT_PREFIXES=\"\;/\"")

set(CXX_SRCS
  parameter_file.cpp
  map_loader/water_map.cpp
  map_loader/map_loader.cpp
  map_loader/imf.cpp
  map_loader/forest.cpp
  map_loader/map_generator.cpp
  effects/factory.cpp
  effects/particle_system.cpp
  ${PROJECT_SOURCE_DIR}/_modules/inih/cpp/INIReader.cpp
  ${PROJECT_SOURCE_DIR}/_modules/inih/ini.c
  ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

add_library(common ${CXX_SRCS})

target_link_libraries(common render_util)
