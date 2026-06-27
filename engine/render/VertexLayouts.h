/******* VertexLayouts.h ***************************************************/ /**
 *
 * @file VertexLayouts.h
 *
 * Vertex memory layout used in the rendering pipeline
 *  
 * @version 1.0
 * @author SpookyProDH-Coder
 * @date 14/06/2026
 ***************************************************************************/

#ifndef _H_VERTEX_LAYOUTS
#define _H_VERTEX_LAYOUTS

#include <bgfx/bgfx.h>

/**
 * @brief Vertex format:
 * - Position (X,Y,Z): 3 * sizeof(float) = 12 bytes
 * - Color (AGBR):     1 * sizeof(uint32_t) = 4 bytes
 */
struct PosColorVertex 
{
    float x, y, z;  ///< 3D corrdinates (X, Y, Z)
    unsigned abgr;  ///< 32-bit ABGR color (0xAABBGGRR)

    static bgfx::VertexLayout ms_layout;    ///< Shared GPU vertex layout specification

    /** Function init
     * 
     * @brief Initialize static vertex layout registry
     */
    static void init()
    {
        ms_layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }
};

#endif