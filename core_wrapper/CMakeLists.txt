set(library_name ${core_wrapper_library_name})
if(NOT static_core_wrapper)
  set(library_type SHARED)
endif(NOT static_core_wrapper)

if(no_refresh_maps)
  add_definitions(-DNO_REFRESH_MAPS)
endif(no_refresh_maps)

add_definitions(-DINI_ALLOW_MULTILINE=0 -DINI_ALLOW_INLINE_COMMENTS=1 -DINI_INLINE_COMMENT_PREFIXES="//")

include_directories(
  include
  include/jni
  ${PROJECT_SOURCE_DIR}/_modules/inih/cpp
  jni
)

set(CXX_SRCS
  main.cpp
  wgl_wrapper.cpp
  sfs/sfs.cpp
  sfs/hash.cpp
  core/core.cpp
  core/ressource_loader.cpp
  core/map.cpp
  core/render_state.cpp
  core/scene.cpp
  jni_wrapper/jni_wrapper.cpp
  gl_wrapper/wgl_interface.cpp
  gl_wrapper/gl_wrapper_main.cpp
  gl_wrapper/texture_state.cpp
  gl_wrapper/arbprogram.cpp
  ${PROJECT_SOURCE_DIR}/_modules/inih/cpp/INIReader.cpp
  ${PROJECT_SOURCE_DIR}/_modules/inih/ini.c
)

set_source_files_properties(
  sfs/sfs.cpp

  PROPERTIES
#     COMPILE_FLAGS "-O0"
)

set(JNI_WRAPPER_CLASSES
  Camera
  Landscape
  Render
  Renders
  Sun
)


add_executable(jni_generator jni_wrapper/generate_meta_code.cpp)

set(generator_cmd jni_generator)

if(platform_mingw)
  if(NOT CMAKE_HOST_WIN32)
    set(generator_cmd wine jni_generator)
  endif(NOT CMAKE_HOST_WIN32)
endif(platform_mingw)

foreach(class_name ${JNI_WRAPPER_CLASSES})
  set(CXX_SRCS ${CXX_SRCS} jni_wrapper/wrap_${class_name}.cpp)
  set(jni_wrapper_classes_string "${class_name} ${jni_wrapper_classes_string}")

  foreach(cmd definitions registration)
    set(output ${PROJECT_BINARY_DIR}/_generated/jni_wrapper/${class_name}_${cmd})

    add_custom_command(
        OUTPUT ${output}
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/_generated/jni_wrapper
        COMMAND ${generator_cmd} ${cmd} ${class_name}
          < ${CMAKE_CURRENT_SOURCE_DIR}/jni_wrapper/signatures
          > ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/jni_wrapper/signatures
        DEPENDS jni_generator
    )
    set(generated_output ${generated_output} ${output})
  endforeach(cmd)
endforeach(class_name)

foreach(name table definitions)
  set(output ${PROJECT_BINARY_DIR}/_generated/jni_wrapper/registrator_${name})
  add_custom_command(
      OUTPUT ${output}
      COMMAND mkdir -p ${PROJECT_BINARY_DIR}/_generated/jni_wrapper
      COMMAND ${generator_cmd} registrator-${name} ${jni_wrapper_classes_string} > ${output}
      DEPENDS jni_generator
  )
  set(generated_output ${generated_output} ${output})
endforeach(name)

add_custom_target(core_wrapper_generated DEPENDS ${generated_output})



add_library(${library_name} ${library_type} ${CXX_SRCS})

add_dependencies(${library_name} core_wrapper_generated)

target_link_libraries(${library_name}
  common
  render_util
)

if(platform_wine AND NOT static_core_wrapper)
  target_link_libraries(${library_name}
    -Wb,--export=${CMAKE_CURRENT_SOURCE_DIR}/il2_core_wrapper.spec
  )
endif(platform_wine AND NOT static_core_wrapper)

if(NOT static_core_wrapper)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
    DESTINATION ${il2ge_lib_dir}
    RENAME ${library_name}.dll
  )
endif(NOT static_core_wrapper)
