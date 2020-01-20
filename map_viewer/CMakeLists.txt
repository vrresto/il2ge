set(SRCS
  map_loader_dump.cpp
  main.cpp
)

if (platform_mingw)
  set(SRCS ${SRCS} ${PROJECT_SOURCE_DIR}/common/exception_handler_win32.cpp)
endif()

add_executable(map_viewer ${SRCS})

target_link_libraries(map_viewer
  common
  viewer
)

if (platform_mingw)
  target_link_libraries(map_viewer ole32)
endif()

install(TARGETS map_viewer
  DESTINATION .
)
