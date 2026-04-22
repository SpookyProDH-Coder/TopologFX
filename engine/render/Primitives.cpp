#include <vector>

#include "Primitives.h"
#include "VertexLayouts.h"

using namespace std;

bgfx::VertexLayout PosColorVertex::ms_layout;

Mesh Primitives::CreateCube() 
{
    PosColorVertex::init();

    static const PosColorVertex s_cubeVertices[] = {
        {-1.0f,  1.0f,  1.0f, 0xff0000ff }, { 1.0f,  1.0f,  1.0f, 0xff00ff00 },
        {-1.0f, -1.0f,  1.0f, 0xffff0000 }, { 1.0f, -1.0f,  1.0f, 0xff00ffff },
        {-1.0f,  1.0f, -1.0f, 0xffff00ff }, { 1.0f,  1.0f, -1.0f, 0xffffff00 },
        {-1.0f, -1.0f, -1.0f, 0xffffffff }, { 1.0f, -1.0f, -1.0f, 0xff000000 },
    };

    static const uint16_t s_cubeIndices[] = {
        0, 1, 2, 1, 3, 2,
        4, 6, 5, 5, 6, 7,
        0, 2, 4, 4, 2, 6,
        1, 5, 3, 5, 7, 3,
        0, 4, 1, 4, 5, 1,
        2, 3, 6, 6, 3, 7
    };

    Mesh mesh;
    mesh.vbh = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_layout);
    mesh.ibh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices)));
    return mesh;
}

Mesh Primitives::CreateFromMetricSpace(const MetricSpace& ms, float epsilon) 
{
    PosColorVertex::init();
    const auto& setX = ms.getSet();
    unsigned i, j, numPoints = setX.count;

    // 1. Crear Vértices (Átomos)
    vector<PosColorVertex> vertices(numPoints);
    for (i = 0; i < numPoints; i++) 
    {
        vertices[i].x = setX.elements[i].x;
        vertices[i].y = setX.elements[i].y;
        vertices[i].z = setX.elements[i].z;
        vertices[i].abgr = 0xffffff00;
    }

    vector<unsigned> indices;
    for (i = 0; i < numPoints; ++i) 
    {
        for (j = i + 1; j < numPoints; ++j) 
        {
            if (ms.distance(i, j) < epsilon) 
            {
                indices.push_back((uint16_t)i);
                indices.push_back((uint16_t)j);
            }
        }
    }

    Mesh mesh;
    mesh.vbh = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), sizeof(PosColorVertex) * vertices.size()), PosColorVertex::ms_layout);
    mesh.ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), sizeof(unsigned) * indices.size()));
    
    cout << "[DEBUG] Created mesh with " << indices.size() << " indices." << endl;

    return mesh;
}