set(CXX_SRCS
  water_map.cpp
  map_loader.cpp
  imf.cpp
  forest.cpp
  map_generator.cpp
)

add_library(common ${CXX_SRCS})

target_link_libraries(common render_util)
