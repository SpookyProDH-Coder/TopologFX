#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11  // Añadimos soporte para X11
#include <wayland-client.h>
#include <wayland-egl.h>
#include <X11/Xlib.h>          // Header de X11

#include <wayland-client.h>
#include <wayland-egl.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/math.h>
#include <bx/allocator.h>

#include "core/Registry.h"
#include "render/Renderer.h"
#include "render/Primitives.h"
#include "../topology/MetricSpace.h"

using namespace std;



void InitPresets();
bgfx::PlatformData GeneratePd(void*, void*);
bgfx::ShaderHandle loadShader(const char*);
bgfx::ProgramHandle loadProgram(const char*, const char*);

/** Constant variables */
const unsigned WIDTH = 1280;
const unsigned HEIGHT = 720;

int main(void)
{
	cout << "[*] Starting engine..." << endl;

	cout << "[*] Initiating XInitThreads..." << endl;
	XInitThreads();

	/* Inicializamos GLFW */
	if (!glfwInit())
	{
		cerr << "[!] Error: Couldn't initialize GLFW. Aborting." << endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Demo", nullptr, nullptr);
	if(!window)
	{
		cerr << "[!] Error: Couldn't create GLFW window. Aborting." << endl;
		return -1;
	}

	void *nativeDisplay, *nativeWindow;
	nativeDisplay = nullptr;
	nativeWindow = nullptr;

	nativeDisplay = (void*)glfwGetWaylandDisplay();
	nativeWindow = (void*)glfwGetWaylandWindow(window);
	//#endif

	std::cout << "[DEBUG] nativeDisplay=" << nativeDisplay
		  << " nativeWindow=" << nativeWindow << std::endl;

	bgfx::PlatformData pd = GeneratePd(nativeWindow, nativeDisplay);

	cout << "[DEBUG] Presets initialized." << endl;
	
	Renderer renderer;
	if (!renderer.init(pd, WIDTH, HEIGHT))
	{
		cerr << "[!] Error: Renderer init failed. Aborting." << endl
		<< "This could be caused by using an unknown OS" << endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	InitPresets();
	
	bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);
	renderer.applyLighting(0);
	
	RenderCommand cmd{};
	cmd.entity_id = 2;

	cout << "[*] Engine initialized." << endl;

	//for(unsigned i = 0; i < 3; i++)
	double xpos, ypos, lastX, lastY;
	float dx, dy;
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(window, &lastX, &lastY);

	while(!glfwWindowShouldClose(window))
	{
		glfwGetCursorPos(window, &xpos, &ypos);
		dx = (float)(xpos - lastX);
    	dy = (float)(ypos - lastY);
		lastX = xpos; lastY = ypos;

		bool keys[6] = {
			glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS,
			glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS,
			glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS,
			glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS,
			glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS,
			glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS
		};
		renderer.updateCamera(dx, dy, keys);

		glfwPollEvents();
		renderer.beginFrame();
		renderer.submit(cmd);
		renderer.endFrame();
	}
	cout << "[*] Shutting down engine..." << endl;
	renderer.shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	cout << "[*] Engine stopped." << endl;
	return 0;
}

void InitPresets()
{
	Registry& r = GetRegistry();
	static bx::DefaultAllocator allocator;

    MetricSpace* mySpace = new MetricSpace(&allocator, 10000, 1);
    
	unsigned n = 50;
    mySpace->generate(n, 2);

    // 2. Generar Mesh desde el espacio (epsilon = 3.0 unidades de distancia)
    // Puntos a menos de 3.0 se conectarán con una línea
    r.meshes.insert(2, Primitives::CreateFromMetricSpace(*mySpace, 0.8f));

    // 3. Configurar Material y Entidad
    Material topoMat;
    topoMat.program = loadProgram("shaders/vs_basic.bin", "shaders/fs_basic.bin");
    topoMat.color[0] = 0.2;
	topoMat.color[1] = 0.8; 
	topoMat.color[2] = 1.0;
	topoMat.color[3] = 1.0;
    r.materialModel.insert(2, topoMat);

    Entity topoEnt;
    topoEnt.mesh_id = 2;
    topoEnt.material_id = 2;
    bx::mtxIdentity(topoEnt.transform);
    r.entities.insert(2, topoEnt);

	LightingPreset day{};
	day.clearColor = 0x00010FF;
	day.intensity = 1.9f;
	r.lightingPreset.insert(0u, day);

	LightingPreset night{};
	night.clearColor = 0x000010FF;
	night.intensity = 0.3f;
	r.lightingPreset.insert(1u, night);
}

bgfx::PlatformData GeneratePd(void* window_fw, void* display_fw)
{
	bgfx::PlatformData pd{};

	wl_surface* surface = (wl_surface*)window_fw;
	wl_display* display = (wl_display*)display_fw;

	wl_egl_window* eglWindow = wl_egl_window_create(surface, WIDTH, HEIGHT);
	pd.nwh = eglWindow;
	pd.ndt = display;

	cout << "[DEBUG] At GeneratePd: nwh=" << pd.nwh << " ndt=" << pd.ndt << endl;

	if (!window_fw || !display_fw)
    	cerr << "[!] Error: Wayland not detected" << endl;

	return pd;
}

bgfx::ShaderHandle loadShader(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        std::cerr << "[!] Cannot open shader: " << path << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory* mem = bgfx::alloc(size);
    fread(mem->data, 1, size, file);
    fclose(file);

    return bgfx::createShader(mem);
}

bgfx::ProgramHandle loadProgram(const char* vsPath, const char* fsPath)
{
    bgfx::ShaderHandle vsh = loadShader(vsPath);
    bgfx::ShaderHandle fsh = loadShader(fsPath);

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        std::cerr << "[!] Shader load failed." << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    return bgfx::createProgram(vsh, fsh, true);
}
