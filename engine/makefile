# --- Compiler & Flags configuration  ---
# Made By: SpookyProDH-Coder
CXX      := g++
INCLUDES := -Icore \
			-Irender \
			-Ithird_party/bx/include \
			-Ithird_party/bimg/include \
			-Ithird_party/bgfx/include \
			-Ithird_party/bgfx/3rdparty/dear-imgui \
			-Ithird_party/bgfx/3rdparty/dear-imgui/backends \
		-Ithird_party/bgfx/examples/common \
		-Ithird_party/bgfx/3rdparty \
		-Ithird_party/bimg/3rdparty

CXXFLAGS := -std=c++23 -O3 -Wall -Wextra $(INCLUDES) \
			-DBX_CONFIG_DEBUG=0 \
			-D__STDC_CONSTANT_MACROS \
			-D__STDC_FORMAT_MACROS \

# --- Bgfx tools ---
SHADERC := ./third_party/bgfx/.build/linux64_gcc/bin/shadercRelease

# --- Libraries ---
LDLIBS  := \
	third_party/bgfx/.build/linux64_gcc/bin/libbgfxRelease.a \
	third_party/bgfx/.build/linux64_gcc/bin/libbxRelease.a \
	third_party/bgfx/.build/linux64_gcc/bin/libbimgRelease.a \
	-lglfw -lGL -lX11 -lwayland-egl -lwayland-client -lpthread -ldl

# --- Routes & Files ---
CORE_DIR := core
RENDER_DIR := render
SHADERS_DIR := shaders
TOPOLOGY_DIR := ../topology

IMGUI_SRCS := third_party/bgfx/3rdparty/dear-imgui/imgui.cpp \
			  third_party/bgfx/3rdparty/dear-imgui/imgui_draw.cpp \
			  third_party/bgfx/3rdparty/dear-imgui/imgui_tables.cpp \
			  third_party/bgfx/3rdparty/dear-imgui/imgui_widgets.cpp \

BGFX_IMGUI_SRC := third_party/bgfx/examples/common/imgui/imgui.cpp

SRCS := main.cpp \
		$(RENDER_DIR)/Renderer.cpp \
		$(RENDER_DIR)/Primitives.cpp \
		$(TOPOLOGY_DIR)/MetricSpace.cpp \
		$(CORE_DIR)/SceneManager.cpp \
		$(CORE_DIR)/EntityInspector.cpp \
		$(CORE_DIR)/utils/EngineConsole.cpp \
		$(IMGUI_SRCS) \
		$(BGFX_IMGUI_SRC)
		
OBJS := $(SRCS:.cpp=.o)

VARYING_DEF := $(SHADERS_DIR)/varying.def.sc

VS_SOURCES := $(wildcard $(SHADERS_DIR)/vs_*.sc)
FS_SOURCES := $(wildcard $(SHADERS_DIR)/fs_*.sc)

SHADER_BINS := $(VS_SOURCES:.sc=.bin) $(FS_SOURCES:.sc=.bin)

TARGET := engine.out

# --- Main Rules ---

all: $(SHADER_BINS) $(TARGET)

$(TARGET): $(OBJS)
	@echo "[*] Enlazando ejecutable: $@"
	$(CXX) $(OBJS) -o $@ $(LDLIBS)

%.o: %.cpp
	@echo "[*] Compilando objeto: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Shader Compilation rules ---

# Profile '120': OpenGL 2.1
# Profile '430' or '150': Modern GL

$(SHADERS_DIR)/vs_%.bin: $(SHADERS_DIR)/vs_%.sc $(VARYING_DEF)
	@echo "[*] Compilando Vertex Shader: $<"
	$(SHADERC) -f $< -o $@ \
		--type vertex \
		--platform linux \
		-p 120 \
		-i third_party/bgfx/src \
		-i third_party/bgfx/examples/common \
		--varyingdef $(VARYING_DEF)

$(SHADERS_DIR)/fs_%.bin: $(SHADERS_DIR)/fs_%.sc $(VARYING_DEF)
	@echo "[*] Compilando Fragment Shader: $<"
	$(SHADERC) -f $< -o $@ \
		--type fragment \
		--platform linux \
		-p 120 \
		-i third_party/bgfx/src \
		-i third_party/bgfx/examples/common \
		--varyingdef $(VARYING_DEF)

clean:
	rm -f $(OBJS) $(TARGET) $(SHADERS_DIR)/*.bin

.PHONY: all clean