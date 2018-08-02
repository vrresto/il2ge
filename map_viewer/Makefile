EXECUTABLE_NAME:= map_viewer
PLATFORM= native

include ../paths.mk

BASE_BUILD_DIR:= ../build/$(PLATFORM)
BUILD_DIR:= $(BASE_BUILD_DIR)/map_viewer
COMMON_BUILD_DIR:= $(BASE_BUILD_DIR)/common

include $(RENDER_UTIL_DIR)/cflags.mk

LDFLAGS+= -L$(COMMON_BUILD_DIR)
LDFLAGS+= -L$(VIEWER_BUILD_DIR)
LDFLAGS+= -L$(ENGINE_BUILD_DIR)
LDFLAGS+= -L$(GL_WRAPPER_BUILD_DIR)
LDFLAGS+= -L$(ENGINE_BASE_BUILD_DIR)/libsoil/install/lib

INCLUDES+= -I../include
INCLUDES+= -I$(RENDER_UTIL_DIR)/include
INCLUDES+= -I$(BASE_BUILD_DIR)
INCLUDES+= -I$(ENGINE_BASE_BUILD_DIR)
INCLUDES+= -I$(ENGINE_BASE_BUILD_DIR)/libsoil/install/include

##########################################################################

COMMON_SRCS+= map_loader_dump.cpp
COMMON_SRCS+= main.cpp

ALL_SRCS:= $(COMMON_SRCS)

##########################################################################

OBJS:= $(COMMON_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.o)
DEPS:= $(ALL_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)

.SECONDEXPANSION:
.PHONY: all clean run libcommon librender_util libviewer maps

all: $(BUILD_DIR)/$(EXECUTABLE_NAME)

librender_util:
	make -C $(RENDER_UTIL_DIR)/src
	
libviewer:
	make -C $(RENDER_UTIL_DIR)/viewer

maps:
	make -C $(RENDER_UTIL_DIR)/tools


libcommon:
	make -C ../common

-include $(DEPS)
include $(RENDER_UTIL_DIR)/rules.mk


$(BUILD_DIR)/$(EXECUTABLE_NAME): $(OBJS) libcommon libviewer librender_util maps
	$(CXX) -o $@  $(LDFLAGS) $(OBJS) -lcommon -lviewer -lrender_util -lgl_wrapper -lglfw -lGL -ltiff


##########################################################################

run: $(BUILD_DIR)/$(EXECUTABLE_NAME)
	$<
