include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

include(interface_version.cmake)

set(jniclass_code ${CMAKE_CURRENT_BINARY_DIR}/jniclass_code.i)
set(interface_version_inc ${CMAKE_CURRENT_BINARY_DIR}/interface_version.inc)

configure_file(jniclass_code.i.in ${jniclass_code})
configure_file(interface_version.inc.in ${interface_version_inc})

set(swig_input
  GraphicsExtender.i
  module_code.i
  interface.h
  ${jniclass_code}
  ${interface_version_inc}
)

set(swig_output_dir ${CMAKE_CURRENT_BINARY_DIR}/output)

set(swig_output_cxx ${swig_output_dir}/GraphicsExtender_wrap.cxx)
set(swig_output_java
  ${swig_output_dir}/GraphicsExtenderJNI.java
  ${swig_output_dir}/GraphicsExtender.java
)

add_custom_target(create_swig_output_dir
    COMMAND mkdir -p ${swig_output_dir}
)

add_custom_command(
    OUTPUT ${swig_output_cxx} ${swig_output_java}
    COMMAND swig -outcurrentdir
      -package com.maddox.il2ge
      -I${CMAKE_CURRENT_SOURCE_DIR}
      -I${CMAKE_CURRENT_BINARY_DIR}
      -java -c++ ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsExtender.i
    WORKING_DIRECTORY ${swig_output_dir}
    DEPENDS ${swig_input}
)

add_custom_target(swig_generate
  DEPENDS ${swig_input} ${swig_output_java} ${swig_output_cxx}
)

set_source_files_properties(interface_wrapper.cpp PROPERTIES
  COMPILE_FLAGS -Wno-missing-declarations
  OBJECT_DEPENDS ${swig_output_cxx}
)

add_library(java_interface STATIC
  interface_version.cpp
  interface_wrapper.cpp
  interface.cpp
)

add_dependencies(java_interface
  swig_generate
)

if(enable_java_dist)
  add_custom_target(java_compile
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${il2ge_data_dir}/java_classes
    COMMAND env CLASSPATH="${java_classpath}" sh compile.sh
  )

  add_custom_target(java_dist
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${il2ge_data_dir}/java_classes
    COMMAND zip -r il2ge_classes.zip com
    DEPENDS java_compile
  )
endif()

install(FILES ${swig_output_java} HotKeys.java
  DESTINATION ${il2ge_data_dir}/java_classes/com/maddox/il2ge
)

install(FILES compile.sh
  DESTINATION ${il2ge_data_dir}/java_classes
)
