set(library_name "il2ge")

include_directories(
  ${PROJECT_SOURCE_DIR}/_modules/inih/cpp
)

set(resource_script ${CMAKE_CURRENT_BINARY_DIR}/resources.rc)
set(resource_obj ${CMAKE_CURRENT_BINARY_DIR}/resources.o)

set(file_description "IL-2 Graphics Extender")
set(file_version "build: ${build_job_id}")
if(enable_debug)
set(file_version "${file_version} (debug)")
endif()

configure_file(resources.rc.in ${resource_script})

set_source_files_properties(
  il2ge.def
  ${resource_obj}
  PROPERTIES EXTERNAL_OBJECT 1
)

add_custom_command(
    OUTPUT ${resource_obj}
    COMMAND ${COMPILER_PREFIX}-windres ${resource_script} ${resource_obj}
    DEPENDS ${resource_script}
)

add_definitions(
  -DCORE_WRAPPER_LIBRARY_NAME="${core_wrapper_library_name}"
  -DIL2GE_USE_CUSTOM_ASSERT
)

set(SRCS
  ${PROJECT_SOURCE_DIR}/common/exception_handler.cpp
  ${resource_obj}
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
