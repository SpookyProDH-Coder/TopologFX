# --- Configuración de Compilador y Flags ---
CXX      := g++
INCLUDES := -Icore \
			-Irender \
			-Ithird_party/bx/include \
			-Ithird_party/bimg/include \
			-Ithird_party/bgfx/include
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 \
            $(INCLUDES) \
			-DBX_CONFIG_DEBUG=0 \
            -D__STDC_CONSTANT_MACROS \
            -D__STDC_FORMAT_MACROS \

# --- Herramientas de bgfx ---
# Asegúrate de que esta ruta sea correcta en tu sistema
SHADERC := ./third_party/bgfx/.build/linux64_gcc/bin/shadercRelease

# --- Librerías ---
LDLIBS  := \
    third_party/bgfx/.build/linux64_gcc/bin/libbgfxRelease.a \
    third_party/bgfx/.build/linux64_gcc/bin/libbxRelease.a \
    third_party/bgfx/.build/linux64_gcc/bin/libbimgRelease.a \
    -lglfw -lGL -lX11 -lwayland-egl -lwayland-client -lpthread -ldl

# --- Archivos y Rutas ---
CORE_DIR := core
RENDER_DIR := render
SHADERS_DIR := shaders
TOPOLOGY_DIR := ../topology

SRCS := main.cpp \
		$(CORE_DIR)/Registry.cpp \
		$(RENDER_DIR)/Renderer.cpp \
		$(RENDER_DIR)/Primitives.cpp \
		$(TOPOLOGY_DIR)/MetricSpace.cpp
		
OBJS := $(SRCS:.cpp=.o)

# Archivo de definición de interfaz (Obligatorio para bgfx)
VARYING_DEF := $(SHADERS_DIR)/varying.def.sc

# Escaneo de shaders
VS_SOURCES := $(wildcard $(SHADERS_DIR)/vs_*.sc)
FS_SOURCES := $(wildcard $(SHADERS_DIR)/fs_*.sc)

# Los binarios se generarán en la misma carpeta de shaders
SHADER_BINS := $(VS_SOURCES:.sc=.bin) $(FS_SOURCES:.sc=.bin)

TARGET := engine_demo

# --- Reglas Principales ---

all: $(SHADER_BINS) $(TARGET)

$(TARGET): $(OBJS)
	@echo "[*] Enlazando ejecutable: $@"
	$(CXX) $(OBJS) -o $@ $(LDLIBS)

%.o: %.cpp
	@echo "[*] Compilando objeto: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Reglas de Compilación de Shaders ---

# Perfil '120' es para OpenGL 2.1 (muy compatible)
# Perfil '430' o '150' si usas características modernas de GL
# -i añade carpetas para encontrar #include <bgfx_shader.sh>

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