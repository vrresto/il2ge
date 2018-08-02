BUILD_DIR:= ../build/loader

DLL_NAME:=dinput.dll

# CROSS_PREFIX:=i686-w64-mingw32-
# CC:=$(CROSS_PREFIX)gcc
# CXX:=$(CROSS_PREFIX)g++

CC:=winegcc -m32
CXX:=wineg++ -m32

CCLD:=$(CXX)

INCLUDES:=-Isrc -Iinclude -I../include

# CFLAGS+= -O2
CFLAGS+= -Wall -Werror
CFLAGS+= -fdata-sections -ffunction-sections
CFLAGS+= -fvisibility=hidden
CFLAGS+= -fno-strict-aliasing

# CFLAGS+= -g
# LDFLAGS+= -Wl,-enable-stdcall-fixup
LDFLAGS+= -s
# LDFLAGS+= -static-libgcc -static-libstdc++
# LDFLAGS+= --gc-sections --print-gc-sections
LDFLAGS+= -lmsvcrt

CFLAGS+= -I$(JDK_PATH)/include

CXXFLAGS:=-std=c++17

C_SRCS+= src/iat.c
CXX_SRCS:=src/main.cpp

OBJS+= $(C_SRCS:%.c=$(BUILD_DIR)/%.c.o)
OBJS+= $(CXX_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.o)
DEPS+= $(C_SRCS:%.c=$(BUILD_DIR)/%.c.d)
DEPS+= $(CXX_SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)

.PHONY: clean all
.SECONDEXPANSION:

all: $(BUILD_DIR)/$(DLL_NAME)

-include $(DEPS)


$(BUILD_DIR)/$(DLL_NAME): $(OBJS) loader.def
	$(CCLD) -o $@ -shared $(LDFLAGS) $^
	cp $@.so $@

$(BUILD_DIR)/%/.dummy:
	mkdir -p $(@D)
	touch $@

$(BUILD_DIR)/.dummy:
	mkdir -p $(@D)
	touch $@

$(BUILD_DIR)/%.c.o: %.c | $$(@D)/.dummy
	$(CC) -o $@ -c -MMD -MP $(INCLUDES) $(CFLAGS) $<

$(BUILD_DIR)/%.cpp.o: %.cpp | $$(@D)/.dummy
	$(CXX) -o $@ -c -MMD -MP $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $<


clean:
	rm -rf $(BUILD_DIR)
