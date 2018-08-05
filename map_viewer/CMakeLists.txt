set(SRCS
  map_loader_dump.cpp
  main.cpp
)

add_executable(map_viewer ${SRCS})

target_link_libraries(map_viewer
  common
  viewer
)
