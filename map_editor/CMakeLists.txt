set(SRCS
#   ${PROJECT_SOURCE_DIR}/common/exception_handler.cpp
  main.cpp
)

add_executable(map_editor ${SRCS})

target_link_libraries(map_editor
  common
  viewer
  ole32
)

install(TARGETS map_editor
  DESTINATION .
)
