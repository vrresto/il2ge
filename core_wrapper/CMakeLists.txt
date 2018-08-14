set(library_name ${core_wrapper_library_name})

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

    set(generator_cmd jni_generator)

    if(platform_mingw)
      if(NOT CMAKE_HOST_WIN32)
        set(generator_cmd wine jni_generator)
      endif(NOT CMAKE_HOST_WIN32)
    endif(platform_mingw)

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


add_custom_target(core_wrapper_generated DEPENDS ${generated_output})

add_library(${library_name} SHARED ${CXX_SRCS})

add_dependencies(${library_name} core_wrapper_generated)

target_link_libraries(${library_name}
  common
  render_util
)

if(platform_wine)
  target_link_libraries(${library_name}
    -Wb,--export=${CMAKE_CURRENT_SOURCE_DIR}/il2_core_wrapper.spec
  )
endif(platform_wine)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
  DESTINATION .
  RENAME ${library_name}.dll
)
