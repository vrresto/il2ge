set(SRCS
  src/iat.c
  src/main.cpp
)

add_library(dinput SHARED ${SRCS})

target_link_libraries(dinput
  msvcrt
  -Wb,--export=${CMAKE_CURRENT_SOURCE_DIR}/loader.def
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dinput${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION .
  RENAME dinput.dll
)
