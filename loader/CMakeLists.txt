set(SRCS
  src/iat.c
  src/main.cpp
)

add_library(dinput SHARED ${SRCS})

target_link_libraries(dinput msvcrt)

set_target_properties(dinput PROPERTIES
  PREFIX ""
  SUFFIX ".dll.so"
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dinput.dll.so
  DESTINATION .
  RENAME dinput.dll
)

