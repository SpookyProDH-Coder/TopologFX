#include <vector>

#include "Primitives.h"
#include "../../topology/TopologyPolicies.h"
#include "VertexLayouts.h"

using namespace std;

bgfx::VertexLayout PosColorVertex::ms_layout;

Mesh Primitives::MS_Plane(const MetricSpace& ms, float size) 
{
    PosColorVertex::init();
    const unsigned WHITE = 0xffffffff;
    const auto& setX = ms.getSet();
    float half = size / 2.0f;
    vector<PosColorVertex> vertex;
    vector<unsigned> indexes;
    float px, py, pz;
    unsigned i, offset;

    for (i = 0; i < setX.count; ++i) 
    {
        px = setX.elements[i].x;
        py = setX.elements[i].y;
        pz = setX.elements[i].z;
        offset = (unsigned)vertex.size();

        vertex.push_back({px-half, py+half, pz, WHITE});
        vertex.push_back({px+half, py+half, pz, WHITE});
        vertex.push_back({px-half, py-half, pz, WHITE});
        vertex.push_back({px+half, py-half, pz, WHITE});

        indexes.push_back(offset + 0); indexes.push_back(offset + 1); indexes.push_back(offset + 2);
        indexes.push_back(offset + 1); indexes.push_back(offset + 3); indexes.push_back(offset + 2);
    }

    return Primitives::buildMesh(vertex, indexes);
}

Mesh Primitives::QT_Space(const MetricSpace& ms, unsigned resolution, const QuotientPolicy& policy, const Embedding<bx::Vec3>& embedding)
{
    ParametricGrid grid(resolution+1, resolution+1);
    unsigned i, j, i0, i1, i2, i3;

    const auto& setX = ms.getSet();
    
    for (i = 0; i < setX.count; i++)
    {
        float u = (float)(i % resolution) / (float)(resolution - 1);
        float v = (float)(i / resolution) / (float)(resolution - 1);

        grid.uv_points.push_back({u, v});
    }

    for (i = 0; i < resolution-1; i++)
    {
        for (j = 0; j < resolution-1; j++)
        {
            i0 = j * (resolution+1) + i;
            i1 = j * (resolution+1) + i+1;
            i2 = (j+1) * (resolution+1) + i;
            i3 = (j+1) * (resolution+1) + i+1;

            grid.indexes.push_back(i0);
            grid.indexes.push_back(i1);
            grid.indexes.push_back(i2);

            grid.indexes.push_back(i1);
            grid.indexes.push_back(i3);
            grid.indexes.push_back(i2);
        }
    }

    policy.applyRule(grid);
    vector<PosColorVertex> final_vertices(setX.count);

    KleinParameters kp;
    MorbiusParameters mp;
    for (i = 0; i < setX.count; i++)
    {
        Vec2 uv = grid.uv_points[i];
        bx::Vec3 pos3D = embedding.map(uv, kp, mp);

        final_vertices[i].x = pos3D.x;
        final_vertices[i].y = pos3D.y;
        final_vertices[i].z = pos3D.z;

        final_vertices[i].abgr = 0xFF8F09FF;
    }
    return Primitives::buildMesh(final_vertices, grid.indexes);
}

Mesh Primitives::buildMesh(const vector<PosColorVertex>& vertex, const vector<unsigned>& indexes)
{
    Mesh mesh;
    mesh.vbh = bgfx::createVertexBuffer(
        bgfx::copy(vertex.data(), sizeof(PosColorVertex) * vertex.size()), 
        PosColorVertex::ms_layout);
    mesh.ibh = bgfx::createIndexBuffer(
        bgfx::copy(indexes.data(), sizeof(unsigned) * indexes.size()),
         BGFX_BUFFER_INDEX32);
    return mesh;
}