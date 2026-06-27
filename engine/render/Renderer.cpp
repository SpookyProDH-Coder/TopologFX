/******* Renderer.cpp ***************************************************/ /**
 *
 * @file Renderer.cpp
 *
 * Minimal renderer implementation
 *  
 * @version 2.0
 * @author SpookyProDH-Coder
 * @date 16/06/2026
 ***************************************************************************/

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <iostream>

#include "Renderer.h"
#include "../core/Registry.h"

/**
 * Native window system global pointers
 */
void *nativeDisplay = nullptr;
void *nativeWindow = nullptr;

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

bool Renderer::init(const bgfx::PlatformData& pd, unsigned width, unsigned height)
{
    assert(width > 0 && height > 0);
    bool ok = true;

    this->m_width = width;
    this->m_height = height;

    /// 1. Initialize bgfx
    bgfx::Init init;
    init.type = bgfx::RendererType::OpenGL;
    init.resolution.width 	= width;
    init.resolution.height 	= height;
    init.resolution.reset 	= BGFX_RESET_VSYNC;
    init.platformData 		= pd;
    init.debug				= true;

    if (!bgfx::init(init))
    {
        cerr << "[!] Error initializing bgfx." << endl;
        ok = false;
    }
    else
    {
        /// 2. Upload uniform variables to shaders
        this->m_u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
        this->m_u_lightParams = bgfx::createUniform("u_lightParams", bgfx::UniformType::Vec4);

        /// 3. Validate uniforms
        if (!bgfx::isValid(m_u_color))
        {
            cerr << "[!] u_color is not valid." << endl;
            ok = false;
        }
        else if (!bgfx::isValid(m_u_lightParams))
        {
            cerr << "[!] u_color is not valid." << endl;
            ok = false;
        }
        cout << "[*] Initializing setupView" << endl;
        setupView();
    }

    return ok;
}

void Renderer::shutdown()
{
    if (bgfx::isValid(m_u_color))
        bgfx::destroy(m_u_color);
    bgfx::destroy(m_u_lightParams);
    bgfx::shutdown();
}

void Renderer::beginFrame()
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx::touch(0);
}

void Renderer::submit(const RenderCommand& cmd)
{
    assert(m_width > 0 && m_height > 0);

    /// 1. Apply affine transformations
    bgfx::setTransform(cmd.transform);
    bgfx::setVertexBuffer(0, cmd.vbh);
    bgfx::setIndexBuffer(cmd.ibh);

    /// 2. Send material color to shader
    bgfx::setUniform(m_u_color, cmd.color);

    /// 3. Define graphics pipeline state
    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_CW & ~BGFX_STATE_CULL_CCW;
    /*uint64_t state = BGFX_STATE_WRITE_RGB 
                   | BGFX_STATE_WRITE_A 
                   | BGFX_STATE_WRITE_Z 
                   | BGFX_STATE_DEPTH_TEST_LESS;*/

    if (cmd.isPrimitiveLines)
        state |= BGFX_STATE_PT_LINES;
    else if (cmd.isPrimitivePoints)
        state |= BGFX_STATE_PT_POINTS;
        
    /// 4. Apply state and submit
    bgfx::setState(state);
    bgfx::submit(0, cmd.program);
}

void Renderer::endFrame()
{
    bgfx::frame();
}

void Renderer::applyLighting(unsigned preset_id)
{
    LightingPreset preset = static_cast<LightingPreset>(preset_id);
    
    /// [x: Ambient; y: Diffuse; z: Specular; w: Intensity]
    float lightParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    switch (preset)
    {
        case LightingPreset::STUDIO_LIGHT:
            m_currentClearColor = 0x303030ff; // Dark gray
            lightParams[0] = 0.3f;
            lightParams[1] = 0.7f;
            lightParams[2] = 0.5f;
            lightParams[3] = 1.0f;
            break;

        case LightingPreset::DARK_MATTER:
            m_currentClearColor = 0x050505ff; // Almost black
            lightParams[0] = 0.1f;
            lightParams[1] = 0.2f;
            lightParams[2] = 0.9f; 
            lightParams[3] = 0.8f;
            break;

        case LightingPreset::NEON_WIREFRAME:
            m_currentClearColor = 0x110022ff; // Dark purple
            lightParams[0] = 0.8f;
            lightParams[1] = 0.0f;
            lightParams[2] = 0.0f;
            lightParams[3] = 1.5f;
            break;
            
        case LightingPreset::TOPOLOGICAL_DEBUG:
            m_currentClearColor = 0x223344ff; // Metallic blue
            lightParams[0] = 1.0f;
            lightParams[1] = 0.0f;
            lightParams[2] = 0.0f;
            lightParams[3] = 1.0f;
            break;
    }

    // Update main framebuffer clear color
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, m_currentClearColor, 1.0f, 0);

    // Send lighting uniform
    if (bgfx::isValid(m_u_lightParams))
        bgfx::setUniform(m_u_lightParams, lightParams);
}

void Renderer::setupView()
{
    assert(m_width > 0 && m_height > 0);

    resize(m_width, m_height);

    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

    const bx::Vec3 at  = { 0.0f, 0.0f,  0.0f };
    const bx::Vec3 eye = { 0.0f, 0.0f, -5.0f };

    float view[16];
    bx::mtxLookAt(view, eye, at);

    bgfx::setViewTransform(0, view, m_proj);
}

void Renderer::updateCamera(float _dx, float _dy, bool _keys[6])
{
    /// 1. Update Euler angles
    m_yaw += _dx * m_mouseSensitivity;
    m_pitch -= _dy * m_mouseSensitivity;

    /// Clamp pitch for preventing Gimbal Lock
    m_pitch = bx::clamp(m_pitch, -bx::kPiHalf + 0.1f, bx::kPiHalf - 0.1f);

    // 2. Calculate the direction of the camera
    bx::Vec3 forward = {
        bx::cos(m_pitch) * bx::sin(m_yaw),
        bx::sin(m_pitch),
        bx::cos(m_pitch) * bx::cos(m_yaw)
    };

    /// 3. Calculate up, right
    bx::Vec3 up = { 0.0f, 1.0f, 0.0f };
    bx::Vec3 right = bx::normalize(bx::cross(up, forward));
    
    /// 4. Apply camera translation for each active key
    if (_keys[0]) m_camPos = bx::add(m_camPos, bx::mul(forward, m_moveSpeed)); // W
    if (_keys[1]) m_camPos = bx::sub(m_camPos, bx::mul(forward, m_moveSpeed)); // S
    if (_keys[2]) m_camPos = bx::sub(m_camPos, bx::mul(right, m_moveSpeed));   // A
    if (_keys[3]) m_camPos = bx::add(m_camPos, bx::mul(right, m_moveSpeed));   // D

    /// 5. Build view matrix
    float view[16];
    bx::Vec3 at = bx::add(m_camPos, forward); 
    bx::mtxLookAt(view, m_camPos, at);

    /// 6. Update view 0 transformations
    bgfx::setViewTransform(0, view, m_proj);
}

void Renderer::resize(unsigned width, unsigned height)
{
    assert(width > 0 && height > 0);

    m_width = width;
    m_height = height;

    /// 1. Reset bgfx backbuffers
    bgfx::reset(m_width, m_height, BGFX_RESET_VSYNC);

    /// 2. Recalculate projection matrix
    bx::mtxProj(m_proj, 60.0f, m_width / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
    
    /// 3. Adjust current draw rectangle to new window dimension
    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
}