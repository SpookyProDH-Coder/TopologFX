/******* Renderer.h ***************************************************/ /**
 *
 * @file Renderer.h
 *
 * Minimal renderer built over bgfx
 *  
 * @version 1.0
 * @author SpookyProDH-Coder
 * @date 14/06/2026
 ***************************************************************************/

#ifndef _RENDERER
#define _RENDERER

#include <bgfx/bgfx.h>
#include "../core/Registry.h"
#include "../core/HashTable.h"

/**
 * @brief Encapsulation of every required state to draw a geometric entity.
 */
struct RenderCommand
{
    bgfx::VertexBufferHandle vbh;		///< Vertex buffer handler
    bgfx::IndexBufferHandle  ibh;		///< Index buffer handler
    bgfx::ProgramHandle      program;	///< GPU shader
    float transform[16];				///< 4x4 transformation matrix
    float color[4];						///< RGBA material color multiplier
    bool isPrimitiveLines = false;		///< Triggers bgfx::StateFlags::TopologyLineList
    bool isPrimitivePoints = false;		///< Triggers bgfx::StateFlags::TopologyPointList
};

/**
 * @brief Principal non-threaded bgfx graphics subsystem.
 */
class Renderer
{
    public:
        Renderer();
        ~Renderer();

        /**
         * @brief Bgfx context and backbuffer resource initializer
         * @param platformData Native window handles and display server context strings
         * @param width Resolution width of the render target
         * @param height Resolution height of the render target
         * @return bool Graphics creation success
         */
        bool init(const bgfx::PlatformData&, unsigned, unsigned);

        /**
         * @brief Shuts down bgfx
         */
        void shutdown();

        /**
         * @brief Begins a new rendering frame context, resetting the internal command queues
         */
        void beginFrame();

        /**
         * @brief Submits a render comand to pipeline.
         * @param RenderCommand Render defined parameters
         */
        void submit(const RenderCommand&);

        /**
         * @brief Ends the current frame.
         */
        void endFrame();

        /**
         * @brief Set a predefined global lighting configuration.
         * @param id LightingPreset enum number.
         */
        void applyLighting(unsigned);

        /**
         * @brief Update internal view matrix using mouse deltas and keystates
         * @param _dx Mouse X delta
         * @param _dy Mouse Y delta
         * @param _keys Pointer to keystates (W,A,S,D)
         */
        void updateCamera(float, float, bool*);

        /**
         * @brief Update viewport resolution
         * @param width New window width
         * @param height New window height
         */
        void resize(unsigned, unsigned);

    private:
        unsigned m_width = 0;			///< Render viewport width
        unsigned m_height = 0;			///< Render viewport height
        bgfx::UniformHandle m_u_color;	///< Fragment shader uniform color handler

        bgfx::UniformHandle m_u_lightParams;		///< Fragment shader uniform light handler
        unsigned m_currentClearColor = 0x303030ff;	///< Background time buffer clear color (RGBA)
        float m_currentIntensity = 1.0f;			///< Global lighting intensity scalar

        /**
         * @brief Computes View and Projection matrices
         */
        void setupView();
        bx::Vec3 m_camPos = {0.0f, 0.0f, -10.0f};	///< 3D coordinate camera position
        
        float m_yaw = 0.0f;						///< Horizontal rotation angle
        float m_pitch = 0.0f;					///< Vertical rotation angle

        float m_moveSpeed = 0.1f;				///< Linear move speed
        float m_mouseSensitivity = 0.01f;		///< Angular rotation multiplier

        float m_proj[16];						///< Projection matrix
};

#endif