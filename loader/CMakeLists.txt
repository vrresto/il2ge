set(library_name "il2ge")

include_directories(
  ${PROJECT_SOURCE_DIR}/_modules/inih/cpp
)

set_source_files_properties(
  il2ge.def
  PROPERTIES EXTERNAL_OBJECT 1
)

add_definitions(
  -DCORE_WRAPPER_LIBRARY_NAME="${core_wrapper_library_name}"
  -DIL2GE_USE_CUSTOM_ASSERT
)

set(SRCS
  ${PROJECT_SOURCE_DIR}/common/exception_handler.cpp
  il2ge.def
  src/iat.c
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

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION .
  RENAME ${library_name}.dll
)
