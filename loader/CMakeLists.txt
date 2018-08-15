add_definitions(
  -DCORE_WRAPPER_LIBRARY_NAME="${core_wrapper_library_name}"
)

set(SRCS
  src/iat.c
  src/main.cpp
)

add_library(dinput SHARED ${SRCS})

target_link_libraries(dinput
  msvcrt
)

if(static_core_wrapper)
  target_link_libraries(dinput
    ${core_wrapper_library_name}
  )
endif(static_core_wrapper)

if(platform_wine)
  target_link_libraries(dinput
    -Wb,--export=${CMAKE_CURRENT_SOURCE_DIR}/loader.def
  )
endif(platform_wine)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dinput${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION .
  RENAME dinput.dll
)
