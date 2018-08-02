install_path=$(HOME)/games/il2
JDK_PATH:=/usr/lib/jvm/default-java

PLATFORM= wine

include ../paths.mk

BASE_BUILD_DIR:= ../build/$(PLATFORM)
BUILD_DIR:= $(BASE_BUILD_DIR)/core_wrapper
COMMON_BUILD_DIR:= $(BASE_BUILD_DIR)/common

dll_basename=il2_core_wrapper
dll_name=$(dll_basename).dll
spec_file=$(dll_basename).spec

include $(RENDER_UTIL_DIR)/cflags.mk

LDFLAGS+= -L$(ENGINE_BUILD_DIR)
LDFLAGS+= -L$(GL_WRAPPER_BUILD_DIR)
LDFLAGS+= -L$(COMMON_BUILD_DIR)
LDFLAGS+= -L$(BUILD_DIR)
LDFLAGS+= -L$(ENGINE_BASE_BUILD_DIR)/libsoil/install/lib

INCLUDES+= -Iinclude
INCLUDES+= -Iini_parser
INCLUDES+= -I../include
INCLUDES+= -I$(BASE_BUILD_DIR)
INCLUDES+= -I$(BUILD_DIR)
INCLUDES+= -I$(RENDER_UTIL_DIR)/include
INCLUDES+= -I$(ENGINE_BASE_BUILD_DIR)
INCLUDES+= -I$(ENGINE_BASE_BUILD_DIR)/libsoil/install/include
INCLUDES+= -I$(JDK_PATH)/include

##########################################################################

CXX_SRCS+= sfs/sfs.cpp
CXX_SRCS+= core/ressource_loader.cpp
CXX_SRCS+= core/map.cpp
CXX_SRCS+= core/render_state.cpp
CXX_SRCS+= core/gl_objects.cpp
CXX_SRCS+= gl_wrapper/wgl_interface.cpp
CXX_SRCS+= gl_wrapper/gl_wrapper_main.cpp
CXX_SRCS+= gl_wrapper/texture_state.cpp
CXX_SRCS+= gl_wrapper/arbprogram.cpp
CXX_SRCS+= ini_parser/INIReader.cpp
CXX_SRCS+= ini_parser/ini.cpp

MAIN_SRCS+= main.cpp
MAIN_SRCS+= jni_wrapper/jni_wrapper.cpp

JNI_WRAPPER_SRCS+= jni_wrapper/wrap_Camera.cpp
JNI_WRAPPER_SRCS+= jni_wrapper/wrap_Landscape.cpp
JNI_WRAPPER_SRCS+= jni_wrapper/wrap_Render.cpp
JNI_WRAPPER_SRCS+= jni_wrapper/wrap_Renders.cpp
JNI_WRAPPER_SRCS+= jni_wrapper/wrap_Sun.cpp

JNI_GENERATOR_SRCS+= jni_wrapper/generate_meta_code.cpp

JNI_WRAPPER_DEPS+= $(JNI_WRAPPER_SRCS:jni_wrapper/wrap_%.cpp=$(BUILD_DIR)/_generated/jni_wrapper/%_definitions)
JNI_WRAPPER_DEPS+= $(JNI_WRAPPER_SRCS:jni_wrapper/wrap_%.cpp=$(BUILD_DIR)/_generated/jni_wrapper/%_registration)

##########################################################################

JNI_GENERATOR_OBJS:= $(JNI_GENERATOR_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.o)
JNI_WRAPPER_OBJS:= $(JNI_WRAPPER_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.o)
MAIN_OBJS:= $(MAIN_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.o)
OBJS:= $(CXX_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.o)

DEPS+= $(CXX_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)
DEPS+= $(MAIN_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)
DEPS+= $(JNI_GENERATOR_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)
DEPS+= $(JNI_WRAPPER_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)

MAIN_OBJS+= $(JNI_WRAPPER_OBJS)


.SECONDEXPANSION:
.PHONY: all clean install run maps libcommon loader generate_jni


all: $(BUILD_DIR)/$(dll_name).so

libcommon:
	make -C ../common PLATFORM=wine

loader:
	make -C ../loader

generate_jni: jni_wrapper/signatures $(BUILD_DIR)/jni_generator.exe | $(BUILD_DIR)/_generated/jni_wrapper/.dummy
	$(BUILD_DIR)/jni_generator.exe all "$(BUILD_DIR)/_generated/jni_wrapper/" < jni_wrapper/signatures


-include $(DEPS)
include $(RENDER_UTIL_DIR)/rules.mk


$(BUILD_DIR)/jni_generator.exe: $(JNI_GENERATOR_OBJS)
	$(CXX) -v -m32 $(CXXFLAGS) $(LDFLAGS) -o $@ $<

$(BUILD_DIR)/_generated/jni_wrapper/%_definitions: jni_wrapper/signatures $(BUILD_DIR)/jni_generator.exe | $(BUILD_DIR)/_generated/jni_wrapper/.dummy
	$(BUILD_DIR)/jni_generator.exe definitions $* < jni_wrapper/signatures > $@

$(BUILD_DIR)/_generated/jni_wrapper/%_registration: jni_wrapper/signatures $(BUILD_DIR)/jni_generator.exe | $(BUILD_DIR)/_generated/jni_wrapper/.dummy
	$(BUILD_DIR)/jni_generator.exe registration $* < jni_wrapper/signatures > $@

$(BUILD_DIR)/sfs/sfs.cpp.o: sfs/sfs.cpp | $(BUILD_DIR)/sfs/.dummy
	$(CXX) -o $@ -c -MMD -MP $(INCLUDES) $(CXXFLAGS) -O0 $<

$(BUILD_DIR)/lib$(dll_basename).a: libcommon $(spec_file) $(OBJS)
	ar -r $@ $(OBJS)

$(BUILD_DIR)/$(dll_name).so: $(JNI_WRAPPER_DEPS) libcommon $(spec_file) $(BUILD_DIR)/lib$(dll_basename).a $(MAIN_OBJS)
	$(CXX) -shared -o $@  -Wl,-no-whole-archive $(LDFLAGS) $(spec_file) $(MAIN_OBJS) $(BUILD_DIR)/lib$(dll_basename).a -lrender_util -lcommon -lgl_wrapper -ltiff -ldl

# 	-no-whole-archive 


##########################################################################

install: $(BUILD_DIR)/$(dll_name).so loader
	cp $(BUILD_DIR)/$(dll_name).so $(install_path)/$(dll_name)
	cp $(SHADER_DIR)/* $(install_path)/ge/shaders/
	cp ../build/loader/dinput.dll $(install_path)/

run: install
	cd $(install_path) && wine il2fb.exe
