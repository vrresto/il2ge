set(library_name "il2ge")

set(build_job_id $ENV{CI_JOB_ID})
set(commit_sha $ENV{CI_COMMIT_SHA})

if(NOT build_job_id)
  set(build_job_id "local")
endif()

if(NOT commit_sha)
  set(commit_sha "unknown")
endif()

configure_file(version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

set_source_files_properties(
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
  ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
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
