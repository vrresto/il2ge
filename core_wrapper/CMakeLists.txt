include_directories(
  include
  ini_parser
  jni
)

set(CXX_SRCS
  main.cpp
  sfs/sfs.cpp
  core/ressource_loader.cpp
  core/map.cpp
  core/render_state.cpp
  core/gl_objects.cpp
  jni_wrapper/jni_wrapper.cpp
  gl_wrapper/wgl_interface.cpp
  gl_wrapper/gl_wrapper_main.cpp
  gl_wrapper/texture_state.cpp
  gl_wrapper/arbprogram.cpp
  ini_parser/INIReader.cpp
  ini_parser/ini.cpp
)

set(JNI_WRAPPER_CLASSES
  Camera
  Landscape
  Render
  Renders
  Sun
)

add_executable(jni_generator jni_wrapper/generate_meta_code.cpp)

foreach(class_name ${JNI_WRAPPER_CLASSES})
  set(CXX_SRCS ${CXX_SRCS} jni_wrapper/wrap_${class_name}.cpp)
  foreach(cmd definitions registration)
    set(output ${PROJECT_BINARY_DIR}/_generated/jni_wrapper/${class_name}_${cmd})
    add_custom_command(
        OUTPUT ${output}
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/_generated/jni_wrapper
        COMMAND jni_generator ${cmd} ${class_name}
          < ${CMAKE_CURRENT_SOURCE_DIR}/jni_wrapper/signatures
          > ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/jni_wrapper/signatures
        DEPENDS jni_generator
    )
    set(generated_output ${generated_output} ${output})
  endforeach(cmd)
endforeach(class_name)


add_custom_target(core_wrapper_generated DEPENDS ${generated_output})

add_library(core_wrapper SHARED ${CXX_SRCS})

add_dependencies(core_wrapper core_wrapper_generated)

target_link_libraries(core_wrapper
  common
  render_util
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/core_wrapper.dll.so
  DESTINATION .
  RENAME core_wrapper.dll
)
