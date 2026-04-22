#ifndef _H_VERTEX_LAYOUTS
#define _H_VERTEX_LAYOUTS
#include <bgfx/bgfx.h>

struct PosColorVertex 
{
    float x, y, z;
    unsigned abgr;

    static bgfx::VertexLayout ms_layout;

    static void init()
    {
        ms_layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }
};

#endif