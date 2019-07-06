include(interface_version.cmake)

set(init_code ${CMAKE_CURRENT_BINARY_DIR}/init_code.i)
set(interface_version_cpp ${CMAKE_CURRENT_BINARY_DIR}/interface_version.cpp)

configure_file(init_code.i.in ${init_code})
configure_file(interface_version.cpp.in ${interface_version_cpp})

set(swig_output_dir ${CMAKE_CURRENT_BINARY_DIR}/output)

set(swig_output_cxx ${swig_output_dir}/il2ge_wrap.cxx)
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
      -I${CMAKE_CURRENT_SOURCE_DIR}/../include -package com.maddox.il2ge
      -I${CMAKE_CURRENT_BINARY_DIR}
      -java -c++ ${CMAKE_CURRENT_SOURCE_DIR}/il2ge.i
    WORKING_DIRECTORY ${swig_output_dir}
    DEPENDS ${swig_deps} create_swig_output_dir
)

add_custom_target(swig DEPENDS ${swig_output_cxx} ${swig_output_java})

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
