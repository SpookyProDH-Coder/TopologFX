/******* Primitives.cpp ***************************************************/ /**
 *
 * @file Primitives.cpp
 *
 * Implementation of geometric and topological primitive generators
 * This file contaions algorithms to map abstract algebraic topology structures
 * (quotient spaces, simplicial complexes) into geometry buffers for GPU
 * 
 * @version 1.0
 * @author SpookyProDH-Coder
 * @date 16/06/2026
 ***************************************************************************/

#include <vector>
#include <cassert>
#include <map>

#include "Primitives.h"
#include "../../topology/TopologyPolicies.h"
#include "VertexLayouts.h"

using namespace std;

/// Initialize bgfx layout descriptor 
bgfx::VertexLayout PosColorVertex::ms_layout;

namespace Primitives
{
    inline uint32_t colorSurface(unsigned id, bool isOrientable = false) 
    {
        uint32_t h = id * 2654435761u;
        
        // Base intensity is in the range ]128, 255[
        uint8_t intensity = 128 + ((h >> 16) & 0x7F);
        
        uint8_t noise = (h >> 8) & 0x3F; 

        // ABGR format: 0xAABBGGRR
        if (isOrientable)
            return 0xFF000000u | (noise << 16) | (noise << 8) | intensity;  // red
        else
            return 0xFF000000u | (intensity << 16) | (noise << 8) | noise; // blue
    }

    inline uint32_t colorDistance(float distance, float max_distance = 15.0f) 
    {
        float t = bx::clamp(distance / max_distance, 0.0f, 1.0f);
        
        // Gradiente: Rojo (0) -> Amarillo -> Verde -> Cyan -> Azul (1)
        uint8_t r = (uint8_t)(bx::max(0.0f, 1.0f - t * 2.0f) * 255.0f);
        uint8_t g = (uint8_t)(bx::max(0.0f, 1.0f - bx::abs(t - 0.5f) * 2.0f) * 255.0f);
        uint8_t b = (uint8_t)(bx::max(0.0f, (t - 0.5f) * 2.0f) * 255.0f);
        
        return 0xFF000000 | (b << 16) | (g << 8) | r; // ABGR format
    }

    Mesh Surface(const MetricSpace& space, unsigned U, unsigned V, bool useHeatmap = false)
    {
        unsigned totalUniqueVertices, v, u, logicalGridIdx, contiguousVertexId, numIndices, offset;
        unsigned *indices;
        const TopologyPolicies::SimplicialComplex& complex = space.getComplex(); 
        TopologyPolicies::EmbeddingFunction embedding = space.getPolicy().getEmbedding();
        TopologyPolicies::Vec3 coords;

        ///< Total number of unique points after applying the quotient topology.
        totalUniqueVertices = space.getSet().count;
        
        if (totalUniqueVertices == 0)
            return Mesh();

        U = complex.gridU;
        V = complex.gridV;

        bx::AllocatorI* allocator = space.getSet().allocator;

        PosColorVertex* vertices = static_cast<PosColorVertex*>(bx::alloc(allocator, totalUniqueVertices * sizeof(PosColorVertex)));
        bool* posInitialized = static_cast<bool*>(bx::alloc(allocator, totalUniqueVertices * sizeof(bool)));

        /// 2. Index Isomorphism Map : X/~ -> Continuous GPU memory range [0...N-1]
        bx::memSet(posInitialized, 0, totalUniqueVertices * sizeof(bool));

        /// 4. Chromatic discriminant of complex's orientability pixel drawing (funky name haha)
        bool isOrientable = complex.isOrientable;

        /// 3. Parametric Inmersion Geometrization of the compact mainfold.

        float pv, pu;

        for (v = 0; v <= V; v++) 
        {
            pv = 2.0f * ((float)v / V) - 1.0f;

            for (u = 0; u <= U; u++) 
            {
                logicalGridIdx = v * (U + 1) + u;
                contiguousVertexId = complex.quotientMap[logicalGridIdx];

                if (!posInitialized[contiguousVertexId]) 
                {
                    pu = 2.0f * ((float)u / U) - 1.0f;

                    coords = embedding(pu, pv);
                    
                    vertices[contiguousVertexId].x = coords.x;
                    vertices[contiguousVertexId].y = coords.y;
                    vertices[contiguousVertexId].z = coords.z;

                    if (useHeatmap)
                        vertices[contiguousVertexId].abgr = colorDistance(space.distance(0, contiguousVertexId), 10.0f);
                    else
                        vertices[contiguousVertexId].abgr = colorSurface(contiguousVertexId, isOrientable);
                        
                    posInitialized[contiguousVertexId] = true;
                }
            }
        }
        bx::free(allocator, posInitialized);

        numIndices = complex.triangles.size() * 3;
        indices = static_cast<unsigned*>(bx::alloc(allocator, numIndices * sizeof(unsigned)));
        offset = 0;

        for (const TopologyPolicies::Simplex2D& tri : complex.triangles) 
        {
            indices[offset++] = tri.v0;
            indices[offset++] = tri.v1;
            indices[offset++] = tri.v2;
        }

        return buildMesh(vertices, totalUniqueVertices, indices, numIndices, allocator);
    }

    Mesh buildMesh(const PosColorVertex* vertices, unsigned numVertices, const unsigned* indices, unsigned numIndices, bx::AllocatorI* allocator)
    {
        Mesh mesh;

        const bgfx::Memory* vMem = bgfx::makeRef(
            vertices, numVertices * sizeof(PosColorVertex),
            [](void* _ptr, void* _userData) {bx::free((bx::AllocatorI*)_userData, _ptr);},
            allocator
        );

        const bgfx::Memory* iMem = bgfx::makeRef(
            indices, numIndices * sizeof(unsigned),
            [](void* _ptr, void* _userData) { bx::free((bx::AllocatorI*)_userData, _ptr); },
            allocator
        );

        mesh.vbh = bgfx::createDynamicVertexBuffer(vMem, PosColorVertex::ms_layout);
        mesh.ibh = bgfx::createDynamicIndexBuffer(iMem, BGFX_BUFFER_INDEX32);

        return mesh;
    }

    void updateDynamicMesh(Mesh& mesh, const PosColorVertex* new_vertices, unsigned numVertices, bx::AllocatorI* allocator)
    {
        assert(!bgfx::isValid(mesh.vbh));

        const bgfx::Memory* vMem = bgfx::makeRef(
            new_vertices, numVertices * sizeof(PosColorVertex),
            [](void* _ptr, void* _userData) { bx::free((bx::AllocatorI*)_userData, _ptr); },
            allocator
        );

        bgfx::update(mesh.vbh, 0, vMem);
    }
}