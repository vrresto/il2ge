set(library_name "il2ge")

add_definitions(
  -DCORE_WRAPPER_LIBRARY_NAME="${core_wrapper_library_name}"
)

set(SRCS
  src/iat.c
  src/main.cpp
)

add_library(${library_name} SHARED ${SRCS})

if(static_core_wrapper)
  target_link_libraries(${library_name}
    ${core_wrapper_library_name}
  )
endif(static_core_wrapper)

if(platform_wine)
  target_link_libraries(${library_name}
    -Wb,--export=${CMAKE_CURRENT_SOURCE_DIR}/loader.def
  )
endif(platform_wine)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION ${il2ge_data_dir}
  RENAME ${library_name}.dll
)
