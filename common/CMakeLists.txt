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
)

add_library(common ${CXX_SRCS})

target_link_libraries(common render_util)
