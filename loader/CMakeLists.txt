set(library_name "il2ge")

set_source_files_properties(
  forwarder.def
  loader.def
  PROPERTIES EXTERNAL_OBJECT 1
)

add_definitions(
  -DCORE_WRAPPER_LIBRARY_NAME="${core_wrapper_library_name}"
)

set(SRCS
  loader.def
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

target_link_libraries(${library_name}
  -Wl,--enable-stdcall-fixup
  -Wl,--add-stdcall-alias
)

add_library(dinput SHARED forwarder.def src/dummy_entry.c)

target_link_libraries(dinput
  -nostartfiles
  -nostdlib
  -s
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION ${il2ge_data_dir}
  RENAME ${library_name}.dll
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dinput${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION ${il2ge_data_dir}
)
