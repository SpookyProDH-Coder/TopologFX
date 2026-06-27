/******* main.cpp ***************************************************/ /**
 *
 * @file main.cpp
 *
 * Principal program engine
 *
 * @version 2.0
 * @author SpookyProDH-Coder
 * @date 12/06/2026
 ***************************************************************************/

#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11
#include <wayland-client.h>
#include <wayland-egl.h>
#include <X11/Xlib.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <imgui.h>

#include <bx/math.h>
#include <bx/allocator.h>

#include "core/Registry.h"
#include "core/SceneManager.h"
#include "core/EntityInspector.h"
#include "core/CustomImGuiAllocator.h"
#include "core/utils/EngineConsole.h"

#include "render/Renderer.h"
#include "render/Primitives.h"

#include "../topology/MetricSpace.h"

using namespace std;

bgfx::PlatformData GeneratePd(GLFWwindow*, unsigned, unsigned);

void imguiCreate(float _fontSize = 18.0f, bx::AllocatorI* _allocator = NULL);
void imguiDestroy();
void imguiBeginFrame(int32_t _mx, int32_t _my, uint8_t _button, int32_t _scroll, uint16_t _width, uint16_t _height, int32_t _inputChar = -1, bgfx::ViewId _viewId = 255);
void imguiEndFrame();

void ObtainRenderData(unsigned&, unsigned&);

/** Constant variables */
const unsigned WIDTH = 1280;
const unsigned HEIGHT = 720;
const bgfx::ViewId IMGUI_VIEW_ID = 255;
const float FONT_SIZE = 18.0f;

bool g_engineKeys[6] = { false };
ImGuiKey GlfwKeyToImGuiKey(int);
void glfw_char_callback(GLFWwindow*, unsigned);
void glfw_key_callback(GLFWwindow*, int, int, int, int);
void glfw_window_size_callback(GLFWwindow*, int, int);

int main(void)
{
    cout << "[*] Starting engine..." << endl;
    cout << "[*] Initiating XInitThreads..." << endl;
    XInitThreads();

    if (!glfwInit())
    {
        cerr << "[!] Error: Couldn't initialize GLFW. Aborting." << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "TopologFX", nullptr, nullptr);
    
    if(!window)
    {
        cerr << "[!] Error: Couldn't create GLFW window. Aborting." << endl;
        return -1;
    }

    bgfx::PlatformData pd = GeneratePd(window, WIDTH, HEIGHT);

    Renderer renderer;

    if (!renderer.init(pd, WIDTH, HEIGHT))
    {
        cerr << "[!] Error: Renderer init failed. Aborting." << endl
        << "This could be caused by using an unknown OS" << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    SceneManager Scene(&renderer);

    cout << "[*] Initializing Dear ImGui." << endl;
    cout << "[DEBUG] Renderer technology: " << bgfx::getRendererName(bgfx::getRendererType()) << endl;

    IMGUI_CHECKVERSION();
    static CustomImGuiAllocator alloc;
    imguiCreate(FONT_SIZE, &alloc);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // ObtainRenderData return-values inside main loop
    static unsigned total_vertices = 0, total_indices = 0;

    /** GLFW callback functions */
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetCharCallback(window, glfw_char_callback);
    glfwSetWindowUserPointer(window, &renderer);
    glfwSetWindowSizeCallback(window, glfw_window_size_callback);

    cout << "[*] Engine initialized." << endl;

    /** Building scene manager */
    Scene.loadScene();
    Scene.initializeTopologicScene();

    EntityInspector inspector(&GetRegistry());

    unsigned selected_ent = 0;

    EngineConsole console(&Scene, &selected_ent);
    static bool show_console = false;
    static bool tilde_was_pressed = false;

    double xpos, ypos, lastX, lastY;
    float dx, dy;
    
    glfwGetCursorPos(window, &lastX, &lastY);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    static bool camera_paused = false;

    // Auxiliar variable for obtaining the actual frame keypress
    static int32_t typed_char = -1;
    glfwSetCharCallback(window, [](GLFWwindow*, unsigned codepoint) {
        typed_char = (int32_t)codepoint;
    });

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glfwGetCursorPos(window, &xpos, &ypos);

        imguiBeginFrame((int32_t)xpos, (int32_t)ypos, 
                        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? 1 : 0, 
                        0, WIDTH, HEIGHT, typed_char, IMGUI_VIEW_ID);
        typed_char = -1;

        /* GUI */
        ImGui::Begin("TopologFX Control Panel");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Draw indexes: %d", total_vertices);
        ImGui::Text("Draw triangles: %d", total_indices / 3);
        ImGui::SliderInt("Selected Entity", (int*)&selected_ent, 0, 4);
        if (ImGui::Button("Reset Scene")) { /* ... */}
        ImGui::End();

        inspector.render(selected_ent);
        console.render(&show_console);

        imguiEndFrame();

        /**
         * Camera logic
         */
        bool is_p_pressed = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
        static bool p_was_pressed = false;

        if (is_p_pressed && !p_was_pressed)
        {
            camera_paused = !camera_paused;
            
            if (camera_paused)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        p_was_pressed = is_p_pressed;

        if (!camera_paused && !io.WantCaptureMouse) 
        {
            glfwGetCursorPos(window, &xpos, &ypos);
            dx = (float)(xpos - lastX);
            dy = (float)(ypos - lastY);
            lastX = xpos; lastY = ypos;
        } 
        else 
        {
            dx = 0; dy = 0;
            glfwGetCursorPos(window, &lastX, &lastY);
        }

        renderer.updateCamera(dx, dy, g_engineKeys);
        
        lastX = xpos; 
        lastY = ypos;

        /**
         * Command prompt display
         */
        bool tilde_is_pressed = (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS);
        if (tilde_is_pressed && !tilde_was_pressed)
            show_console = !show_console;
        tilde_was_pressed = tilde_is_pressed;

        Scene.updateTransforms();
        Scene.updateTopology();
        ObtainRenderData(total_vertices, total_indices);

        renderer.beginFrame();
        Scene.renderEntity(selected_ent);
        
        renderer.endFrame();
    }

    cout << "[*] Shutting down engine..." << endl;

    imguiDestroy();

    renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    cout << "[*] Engine stopped." << endl;
    return 0;
}

/**
 * @param window_fw Native display memory access
 * @param display_fw Native display memory access
 * @return bgfx::PlatformData 
 */
bgfx::PlatformData GeneratePd(GLFWwindow* window, unsigned width, unsigned height)
{
    bgfx::PlatformData pd{};

    #if defined(GLFW_EXPOSE_NATIVE_WAYLAND) && defined(GLFW_EXPOSE_NATIVE_X11)
    int platform = glfwGetPlatform();

    switch (platform)
    {
        case GLFW_PLATFORM_WAYLAND:
        {
            wl_surface* surface = (wl_surface*)glfwGetWaylandWindow(window);
            wl_display* display = (wl_display*)glfwGetWaylandDisplay();

            if (surface && display) 
            {
                wl_egl_window* eglWindow = wl_egl_window_create(surface, width, height);
                pd.nwh = eglWindow;
                pd.ndt = display;
            }
            break;
        }

        case GLFW_PLATFORM_X11:
        {
            pd.nwh = (void*)(uintptr_t)glfwGetX11Window(window);
            pd.ndt = glfwGetX11Display();
            break;
        }

        default:
        {
            cerr << "[!] Error: Platform not supported." << endl;
            break;
        }
    }
    #endif

    cout << "[DEBUG] At GeneratePd: nwh=" << pd.nwh << " ndt=" << pd.ndt << endl;

    return pd;
}

/**
 * @brief A debugging function for obtaining the total frame geometric primitives.
 * 
 * @param [inout] total_vertices The amount of total vertices in the pipeline.
 * @param [inout] total_indices The amount of indices sent to the GPU.
 */
void ObtainRenderData(unsigned& total_vertices, unsigned& total_indices)
{
    const bgfx::Stats* stats = bgfx::getStats();
    unsigned triangles, lines, points;

    if (stats != nullptr) 
    {
        // 1. Obtain the number of primitives by topology
        triangles = (unsigned)stats->numPrims[bgfx::Topology::TriList];
        lines     = (unsigned)stats->numPrims[bgfx::Topology::LineList];
        points    = (unsigned)stats->numPrims[bgfx::Topology::PointList];
        
        // 2. Calculate total indexes sent to GPU
        total_indices = (triangles * 3) + (lines * 2) + points;

        // 3. Estimate processed vertices by render pipeline
        total_vertices = total_indices; 
    }
    else 
    {
        total_vertices = 0;
        total_indices = 0;
    }
}

/**
 * @brief A lookup table for translating GLFW keys to ImGuiKey
 * @param key GLFW's key identifier (i.e. GLFW_KEY_A)
 * @return ImGuiKey The corresponding key value for ImGui
 * @retval ImGuiKey_None if unmapped
 */
ImGuiKey GlfwKeyToImGuiKey(int key)
{
    switch (key)
    {
        case GLFW_KEY_TAB:         return ImGuiKey_Tab;
        case GLFW_KEY_LEFT:        return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT:       return ImGuiKey_RightArrow;
        case GLFW_KEY_UP:          return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN:        return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP:     return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN:   return ImGuiKey_PageDown;
        case GLFW_KEY_HOME:        return ImGuiKey_Home;
        case GLFW_KEY_END:         return ImGuiKey_End;
        case GLFW_KEY_INSERT:      return ImGuiKey_Insert;
        case GLFW_KEY_DELETE:      return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE:   return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE:       return ImGuiKey_Space;
        case GLFW_KEY_ENTER:       return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE:      return ImGuiKey_Escape;
        case GLFW_KEY_KP_ENTER:    return ImGuiKey_KeypadEnter;
        case GLFW_KEY_A:           return ImGuiKey_A;
        case GLFW_KEY_C:           return ImGuiKey_C;
        case GLFW_KEY_V:           return ImGuiKey_V;
        case GLFW_KEY_X:           return ImGuiKey_X;
        case GLFW_KEY_Y:           return ImGuiKey_Y;
        case GLFW_KEY_Z:           return ImGuiKey_Z;
        default:                   return ImGuiKey_None;
    }
}

/**
 * @brief GLFW callback for text input events.
 * @param window The GLFW window that received the event.
 * @param codepoint The Unicode code point of the character.
 */
void glfw_char_callback(GLFWwindow* window, unsigned codepoint)
{
    ImGuiIO& io = ImGui::GetIO();

    // (BMP, U+0001 to U+FFFF)
    if (codepoint > 0 && codepoint < 0x10000)
        io.AddInputCharacter((unsigned)codepoint);
}

/**
 * @brief Callback for physical keyboard events, handle ImGui state and engine input.
 * 
 * @param window GLFW window that received the event.
 * @param key THe physical toggled key.
 * @param scancode System-specific scancode of the key.
 * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
 * @param mods Bitmask of which modifier key was pressed.
 */
void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    
    // 1. Map and forward the primary key event to ImGui
    ImGuiKey imguiKey = GlfwKeyToImGuiKey(key);

    if (imguiKey != ImGuiKey_None)
        io.AddKeyEvent(imguiKey, (action != GLFW_RELEASE));

    // Update modifier key states
    io.AddKeyEvent(ImGuiKey_LeftCtrl,  (mods & GLFW_MOD_CONTROL) != 0);
    io.AddKeyEvent(ImGuiKey_LeftShift, (mods & GLFW_MOD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiKey_LeftAlt,   (mods & GLFW_MOD_ALT) != 0);

    // 2. Check if ImGui captures keyboard context
    if (io.WantCaptureKeyboard)
    {
        // Prevent movement while typing
        for (int i = 0; i < 6; ++i) 
            g_engineKeys[i] = false;
        return; 
    }

    // 3. Route unhandled input to engine/camera movement controls
    if (action == GLFW_PRESS || action == GLFW_RELEASE)
    {
        bool pressed = (action == GLFW_PRESS);
        if (key == GLFW_KEY_W) g_engineKeys[0] = pressed;
        if (key == GLFW_KEY_S) g_engineKeys[1] = pressed;
        if (key == GLFW_KEY_A) g_engineKeys[2] = pressed;
        if (key == GLFW_KEY_D) g_engineKeys[3] = pressed;
    }

    // 4. Handle global application shortcuts
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/**
 * @brief GLFW callback for resizing window.
 * @param window The GLFW window that received the event.
 * @param width Width to resize
 * @param height Height to resize
 */
void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

    if (renderer != nullptr && width > 0 && height > 0)
        renderer->resize((unsigned)width, (unsigned)height);
}