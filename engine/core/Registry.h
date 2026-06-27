 /******* Registry.h ***************************************************/ /**
 *
 * @file Registry.h
 *
 * The resource registry
 * 
 * It includes:
 * - Mesh: Vertex and index buffer handlers
 * - Material parameters
 * - Entity: The physical reference to mesh and material.
 * - ParametricSurface parameters.
 * - Registry: The storage for hash tables.
 * 
 * @version 1
 * @author SpookyProDH-Coder
 * @date 21/06/2026
 ***************************************************************************/

#ifndef _REGISTRY_H
#define _REGISTRY_H

#include "HashTable.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <memory>
#include <vector>

#include "../../topology/TopologyPolicies.h"

/// Forwarded class
class MetricSpace;

enum class SurfaceType : unsigned
{
    TORUS,
    KLEIN_BOTTLE,
    MOBIUS_STRIP,
    REAL_PROJECTIVE_PLANE,
    SPHERE
};

enum class LightingPreset : unsigned 
{
    STUDIO_LIGHT,
    DARK_MATTER,
    NEON_WIREFRAME,
    TOPOLOGICAL_DEBUG
};

struct ParametricSurface 
{
    SurfaceType type = SurfaceType::TORUS;
    int u_subdivisions = 40;
    int v_subdivisions = 40;
    float p_norm = 2.0f;
    bool needs_rebuild = false;
    bool use_heatmap = false;
};

struct Mesh
{
    bgfx::DynamicVertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::DynamicIndexBufferHandle ibh = BGFX_INVALID_HANDLE;
};

struct Material
{
    float color[4];
    float transform[16];

    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_color = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_model = BGFX_INVALID_HANDLE;
};

struct Entity 
{
    unsigned mesh_id;
    unsigned material_id;

    bx::Vec3 position = { 0.0f, 0.0f, 0.0f };
    bx::Vec3 scale    = { 1.0f, 1.0f, 1.0f };
    bx::Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};

    float transform[16];
    
    Entity() : mesh_id(0), material_id(0)
    {
        bx::mtxIdentity(transform);
    }
};

struct Registry
{
    unsigned size = 64;
    
    HashTable<unsigned, Mesh> meshes{size};
    HashTable<unsigned, Material> materialModel{size};
    HashTable<unsigned, Entity> entities{size};
    HashTable<unsigned, ParametricSurface> parametricSurfaces{size};
    
    std::shared_ptr<MetricSpace> metricSpace = nullptr;
};

// Explicit access point
inline Registry& GetRegistry()
{
    static Registry instance;
    return instance;
}

#endif