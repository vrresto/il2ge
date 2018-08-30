set(library_name "il2ge")

set_source_files_properties(
  forwarder.spec
  forwarder.def
  il2ge.def
  PROPERTIES EXTERNAL_OBJECT 1
)

add_definitions(
  -DCORE_WRAPPER_LIBRARY_NAME="${core_wrapper_library_name}"
)

set(SRCS
  il2ge.def
  src/iat.c
  src/backtrace.cpp
  src/main.cpp
)

add_library(${library_name} SHARED ${SRCS})

if(static_core_wrapper)
  target_link_libraries(${library_name}
    ${core_wrapper_library_name}
  )
endif(static_core_wrapper)

if(platform_mingw)
  target_link_libraries(${library_name}
    -Wl,--enable-stdcall-fixup
    -Wl,--add-stdcall-alias
  )
endif(platform_mingw)

if(platform_wine)
  set(forwarder_def forwarder.spec)
else(platform_wine)
  set(forwarder_def forwarder.def)
endif(platform_wine)

add_library(dinput SHARED
  ${forwarder_def}
  src/dummy_entry.c
)

if(platform_mingw)
  target_link_libraries(dinput
    -nostartfiles
    -nostdlib
    -s
  )
endif(platform_mingw)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION ${il2ge_lib_dir}
  RENAME ${library_name}.dll
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dinput${CMAKE_SHARED_LIBRARY_SUFFIX_C}
  DESTINATION ${il2ge_lib_dir}
  RENAME dinput.dll
)
