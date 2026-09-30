# Builds and runs the host-side renderer for the monochrome LVGL demo UI.
#
#   make run     build and render the demo screens as ASCII art (default)
#   make clean   drop all build output
#
# Needs an MSYS2 environment: run from a UCRT64 shell, or from PowerShell with
#   $env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
#   C:\msys64\usr\bin\make.exe -C tools\sim run

REPO     := ../..
LVGL_DIR ?= $(REPO)/.pio/libdeps/esp32/lvgl
U8G2_DIR ?= $(REPO)/.pio/libdeps/esp32/U8g2
BUILD    := build
EXE      := $(BUILD)/mono_sim.exe
PROBE    := $(BUILD)/bitmap_probe.exe

CC  ?= gcc
CXX ?= g++

CPPFLAGS := -DLV_CONF_INCLUDE_SIMPLE \
            -I$(REPO)/include -I$(REPO)/src -I$(LVGL_DIR) -I$(LVGL_DIR)/src
CFLAGS   := -std=gnu11 -O1 -w -MMD -MP
CXXFLAGS := -std=gnu++20 -O1 -w -MMD -MP

# Recursive wildcard: $(call rwildcard,<dir>,<pattern>)
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# LVGL is plain C and does not compile as C++, so it gets its own pass.
# Its bundled hardware drivers are not needed to render off-screen.
LVGL_SRCS := $(filter-out $(LVGL_DIR)/src/drivers/%,$(call rwildcard,$(LVGL_DIR)/src,*.c))
LVGL_OBJS := $(patsubst $(LVGL_DIR)/src/%.c,$(BUILD)/lvgl/%.o,$(LVGL_SRCS))

APP_OBJS := $(BUILD)/app/demo_ui.o $(BUILD)/app/sim_main.o

# The layout probe links the real U8g2 (plain C) against a dummy panel, so its answers
# come from the library itself rather than the simulator's stand-in.
U8G2_SRCS  := $(wildcard $(U8G2_DIR)/src/clib/*.c)
U8G2_OBJS  := $(patsubst $(U8G2_DIR)/src/clib/%.c,$(BUILD)/u8g2/%.o,$(U8G2_SRCS))
PROBE_OBJS := $(BUILD)/app/bitmap_probe.o

# Changing the LVGL configuration invalidates everything.
LV_CONF := $(REPO)/include/lv_conf.h

.PHONY: all run probe clean
all: $(EXE)

run: $(EXE)
	@./$(EXE)

probe: $(PROBE)
	@./$(PROBE)

$(EXE): $(LVGL_OBJS) $(APP_OBJS)
	@mkdir -p $(@D)
	$(CXX) $^ -o $@

$(PROBE): $(LVGL_OBJS) $(U8G2_OBJS) $(PROBE_OBJS)
	@mkdir -p $(@D)
	$(CXX) $^ -o $@

$(BUILD)/u8g2/%.o: $(U8G2_DIR)/src/clib/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/app/bitmap_probe.o: bitmap_probe.cpp $(LV_CONF)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) -I$(U8G2_DIR)/src/clib $(CXXFLAGS) -c $< -o $@

$(BUILD)/lvgl/%.o: $(LVGL_DIR)/src/%.c $(LV_CONF)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/app/demo_ui.o: $(REPO)/src/ui/demo_ui.cpp $(LV_CONF)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD)/app/sim_main.o: sim_main.cpp $(LV_CONF)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD)

# Header dependencies recorded by -MMD, so edits to i1_blit.h and friends rebuild.
-include $(LVGL_OBJS:.o=.d) $(APP_OBJS:.o=.d) $(U8G2_OBJS:.o=.d) $(PROBE_OBJS:.o=.d)

$(LVGL_SRCS):
	$(error LVGL sources not found in $(LVGL_DIR) - run 'pio run' once to fetch dependencies)
