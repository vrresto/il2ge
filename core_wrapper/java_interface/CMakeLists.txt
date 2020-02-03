include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_source_files_properties(interface_wrapper.cpp PROPERTIES
  COMPILE_FLAGS -Wno-missing-declarations
#   OBJECT_DEPENDS ${swig_output_cxx}
)

add_library(java_interface OBJECT
  interface_version.cpp
  interface_wrapper.cpp
  interface.cpp
)

add_dependencies(java_interface
  swig_generate_cxx
)
