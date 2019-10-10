include_directories(
  include
  include/jni
  jni
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(swig)

set(library_name "il2ge")

set(resource_script ${CMAKE_CURRENT_BINARY_DIR}/resources.rc)
set(resource_obj ${CMAKE_CURRENT_BINARY_DIR}/resources.o)

set(file_version "build: ${build_job_id}")
if(enable_debug)
  set(file_version "${file_version} (debug)")
endif()
set(file_description "IL-2 Graphics Extender - ${file_version}")

if(NOT enable_shortcuts)
  set(enable_shortcuts 0)
endif()

if(NOT enable_menu)
  set(enable_menu 0)
endif()

if(NOT enable_configurable_atmosphere)
  set(enable_configurable_atmosphere 0)
endif()

if(NOT enable_configurable_shadows)
  set(enable_configurable_shadows 0)
endif()

configure_file(resources.rc.in ${resource_script})
configure_file(include/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)

set_source_files_properties(
  il2ge.def
  ${resource_obj}
  PROPERTIES EXTERNAL_OBJECT 1
)

set(SRCS
  ${resource_obj}
  il2ge.def
  main/main.cpp
  main/iat.c
  main/core_wrapper.cpp
  main/wgl_wrapper.cpp
  sfs/sfs.cpp
  sfs/hash.cpp
  core/core.cpp
  core/ressource_loader.cpp
  core/map.cpp
  core/render_state.cpp
  core/scene.cpp
  core/effects.cpp
  core/menu.cpp
  jni_wrapper/jni_wrapper.cpp
  gl_wrapper/wgl_interface.cpp
  gl_wrapper/gl_wrapper_main.cpp
  gl_wrapper/texture_state.cpp
  gl_wrapper/arb_program.cpp
  gl_wrapper/framebuffer.cpp
  ${PROJECT_SOURCE_DIR}/common/exception_handler.cpp
)

set(JNI_WRAPPER_CLASSES
  rts.Time
  il2.engine.RenderContext
  il2.engine.Camera
  il2.engine.Landscape
  il2.engine.Render
  il2.engine.Renders
  il2.engine.LightPoint
  il2.engine.GObj
  il2.engine.Eff3D
  il2.fm.Wind
)

add_definitions(-DIL2GE_USE_CUSTOM_ASSERT)
if(no_refresh_maps)
  add_definitions(-DNO_REFRESH_MAPS)
endif(no_refresh_maps)

add_custom_command(
    OUTPUT ${resource_obj}
    COMMAND ${COMPILER_PREFIX}-windres ${resource_script} ${resource_obj}
    DEPENDS ${resource_script}
)

add_executable(jni_generator jni_wrapper/generate_meta_code.cpp)
target_link_libraries(jni_generator render_util_util)

set(generator_cmd jni_generator)

if(platform_mingw)
  if(NOT CMAKE_HOST_WIN32)
    set(generator_cmd wine jni_generator)
  endif(NOT CMAKE_HOST_WIN32)
endif(platform_mingw)

foreach(class_name ${JNI_WRAPPER_CLASSES})
  set(SRCS ${SRCS} jni_wrapper/wrap_${class_name}.cpp)
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

add_custom_target(class-wrappers
  COMMAND mkdir -p ${PROJECT_BINARY_DIR}/_generated/jni_wrapper/class_wrappers
  COMMAND ${generator_cmd} all ${PROJECT_BINARY_DIR}/_generated/jni_wrapper/class_wrappers
    < ${CMAKE_CURRENT_SOURCE_DIR}/jni_wrapper/signatures
  DEPENDS jni_generator
)

add_custom_target(core_wrapper_generated DEPENDS ${generated_output})

add_library(${library_name} SHARED ${SRCS})

add_dependencies(${library_name} core_wrapper_generated)

target_link_libraries(${library_name}
  common
  render_util
  java_interface
)

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
