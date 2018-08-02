LIBNAME:= libcommon.a
PLATFORM= native

include ../paths.mk

BASE_BUILD_DIR:= ../build/$(PLATFORM)
BUILD_DIR:= $(BASE_BUILD_DIR)/common

include $(RENDER_UTIL_DIR)/cflags.mk

INCLUDES+= -I../include
INCLUDES+= -I$(RENDER_UTIL_DIR)/include
INCLUDES+= -I$(BASE_BUILD_DIR)
INCLUDES+= -I$(ENGINE_BASE_BUILD_DIR)
INCLUDES+= -I$(ENGINE_BASE_BUILD_DIR)/libsoil/install/include

##########################################################################

CXX_SRCS+= water_map.cpp
CXX_SRCS+= map_loader.cpp
CXX_SRCS+= imf.cpp
CXX_SRCS+= forest.cpp

OBJS+= $(CXX_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.o)
DEPS+= $(CXX_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)

##########################################################################

.SECONDEXPANSION:
.PHONY: all clean librender_util

all: $(BUILD_DIR)/$(LIBNAME)

librender_util:
	make -C $(RENDER_UTIL_DIR)/src

-include $(DEPS)
include $(RENDER_UTIL_DIR)/rules.mk

$(BUILD_DIR)/$(LIBNAME): librender_util $(OBJS)
	ar -r $@ $(OBJS)


##########################################################################

print_vars:
	@echo PLATFORM: $(PLATFORM)
	@echo deps: $(DEPS)
	@echo CXX: $(CXX)
