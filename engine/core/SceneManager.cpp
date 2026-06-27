 /******* SceneManager.cpp ***************************************************/ /**
 *
 * @file SceneManager.cpp
 *
 * Scene manager class implementation
 * 
 * @version 1
 * @author SpookyProDH-Coder
 * @date 21/06/2026
 ***************************************************************************/

#include "SceneManager.h"
#include <bx/math.h>
#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include "../../topology/TopologyPolicies.h"
#include "../../topology/MetricSpace.h"
#include "../render/Primitives.h"
#include <cstring>
#include <cmath>
#include <numeric>
#include <execution>
#include <immintrin.h>

namespace 
{
    const TopologyPolicies::WordSurfacePolicy* g_engineToTopologyMap[] = {
        &torus,     // 0: SurfaceType::TORUS
        &klein,     // 1: SurfaceType::KLEIN_BOTTLE
        &mobius,    // 2: SurfaceType::MORBIUS_STRIP
        &proj,      // 3: SurfaceType::REAL_PROJECTIVE_PLANE
        &sphere     // 4: SurfaceType::SPHERE
    };
};

SceneManager::SceneManager(Renderer* renderer)
: m_renderer(renderer) 
{
    PosColorVertex::init();
    loadProgram("shaders/vs_basic.bin", "shaders/fs_basic.bin");
}

SceneManager::~SceneManager()
{
    clear();
}

const Registry& SceneManager::getRegistry() const
{
    return GetRegistry(); 
}

void SceneManager::clear()
{
    m_activeEntities.clear();
    m_nextEntityId = 0;
}

unsigned SceneManager::addEntity(const Entity& ent)
{
    unsigned id = m_nextEntityId++;
    GetRegistry().entities.insert(id, ent);
    m_activeEntities.push_back(id);
    return id;
}

void SceneManager::renderEntity(unsigned entity_id)
{
    Entity *ent = GetRegistry().entities.getPointer(entity_id);

    if (ent == nullptr)
    {
        std::cerr << "[!] Entity " << entity_id << " not found." << std::endl;
        return;
    }

    Mesh *mesh = GetRegistry().meshes.getPointer(ent->mesh_id);
    Material *mat = GetRegistry().materialModel.getPointer(ent->material_id);

    if (mesh != nullptr && mat != nullptr)
    {
        bgfx::setTransform(mat->transform);
        
        if (bgfx::isValid(mat->u_color)) {
            bgfx::setUniform(mat->u_color, mat->color);
        }

        // Validación de memoria de GPU para evitar segfaults de geometría
        if (bgfx::isValid(mesh->vbh) && bgfx::isValid(mesh->ibh)) 
        {
            bgfx::setVertexBuffer(0, mesh->vbh);
            bgfx::setIndexBuffer(mesh->ibh);
            
            bgfx::setState(BGFX_STATE_DEFAULT); 
            bgfx::submit(0, mat->program);
        }

        /**
        float mtx[16];
        bx::mtxSRT(mtx, 
            ent->scale.x, ent->scale.y, ent->scale.z,          // Scale
            ent->rotation.x, ent->rotation.y, ent->rotation.z, // Rotation (in radians)
            ent->position.x, ent->position.y, ent->position.z  // Translation
        );
        bgfx::setTransform(mtx);
        bgfx::setVertexBuffer(0, mesh->vbh);
        bgfx::setIndexBuffer(mesh->ibh);
        bgfx::setState(BGFX_STATE_DEFAULT);
        bgfx::submit(0, mat->program);*/
    }
}

void SceneManager::setEntityPosition(unsigned id, const bx::Vec3& pos)
{
    /*
    Entity ent;
    if (GetRegistry().entities.search(id, ent)) 
    {
        ent.position = pos;
        mxSRT_Update(id, ent);
        GetRegistry().entities.insert(id, ent); // Overwrite in the HashTable
    }*/

    Entity* ent = GetRegistry().entities.getPointer(id);
    if (ent != nullptr) 
    {
        ent->position = pos;
        mxSRT_Update(id, *ent);
    }
}

void SceneManager::setEntityScale(unsigned id, const bx::Vec3& scale)
{
    Entity* ent = GetRegistry().entities.getPointer(id);
    if (ent != nullptr) 
    {
        ent->scale = scale;
        mxSRT_Update(id, *ent);
    }
}

void SceneManager::setEntityRotation(unsigned id, const bx::Quaternion& rot)
{
    Entity* ent = GetRegistry().entities.getPointer(id);
    if (ent != nullptr) 
    {
        ent->rotation = rot;
        mxSRT_Update(id, *ent);
    }
}

void SceneManager::setMetricParamP(unsigned id, float p)
{
    ParametricSurface* surface = GetRegistry().parametricSurfaces.getPointer(id);
    if (surface != nullptr)
    {
        surface->p_norm = p;
        surface->needs_rebuild = true; // Esto dispara updateTopology en el siguiente frame
        std::cout << "[DEBUG] Updated p-norm to " << p << " for entity " << id << std::endl;
    }
    else
    {
        std::cerr << "[!] Error: Entity " << id << " has no ParametricSurface component." << std::endl;
    }
    /*
    std::shared_ptr<MetricSpace> space = GetRegistry().metricSpace;
    if (space != nullptr)
    {
        space->setGeometry(reinterpret_cast<const float(*)[3]>(GetRegistry().entities.getPointer(id)->transform), p);

        for (unsigned entity_id : m_activeEntities)
        {
            ParametricSurface* surface = GetRegistry().parametricSurfaces.getPointer(entity_id);
            if (surface != nullptr)
                surface->needs_rebuild = true; 
        }
        std::cout << "[DEBUG] Updated p-norm to " << p << std::endl;
    }
    else
        std::cerr << "[!] Error: No MetricSpace instantiated in Registry." << std::endl;*/
}

void SceneManager::initializeTopologicScene()
{
    unsigned rootId = 0;

    Entity* rootEntity = GetRegistry().entities.getPointer(rootId);;

    if (rootEntity == nullptr)
    {
        std::cerr << "[!] Error: Root entity not found while initializing topologic scene." << std::endl;
        return;
    }

    ParametricSurface surface;
    surface.type = SurfaceType::TORUS;
    surface.u_subdivisions = 40;
    surface.v_subdivisions = 40;
    surface.needs_rebuild = true;
    surface.p_norm = 2.0f;
    surface.use_heatmap = false;
    
    GetRegistry().parametricSurfaces.insert(rootId, surface);

    // Apply default lighting preset
    m_renderer->applyLighting(static_cast<unsigned>(LightingPreset::STUDIO_LIGHT));

    updateTopology();
}

void SceneManager::loadScene() 
{
    
    clear();

    Material topoMat;
    topoMat.program = loadProgram("shaders/vs_basic.bin", "shaders/fs_basic.bin");
    topoMat.color[0] = 0.2f; topoMat.color[1] = 0.2f; 
    topoMat.color[2] = 1.0f; topoMat.color[3] = 1.0f;

    topoMat.u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
    GetRegistry().materialModel.insert(0, topoMat);

    Entity ent;
    ent.mesh_id = 0;
    ent.material_id = 0;
    ent.position = {0,0,0};
    ent.rotation = bx::Quaternion{0, 0, 0, 1.0f};
    ent.scale = {1.0f, 1.0f, 1.0f};
    
    addEntity(ent); // Genera la ID = 0

    /* Old method, should be reimplemented...
    static bx::DefaultAllocator allocator;

    unsigned n = 50, m = 100, id = 0;

    TopologyPolicies::WordSurfacePolicy planarPolicy({{'a', 1}, {'b', 1}, {'a', -1}, {'b', -1}}, 10, 10, GeometricEmbedding::Fallback);
    GetRegistry().metricSpace = std::make_shared<MetricSpace>(&allocator, planarPolicy);

    GetRegistry().metricSpace->buildNorm(2.0f);

    GetRegistry().meshes.insert(id, Primitives::Surface(*GetRegistry().metricSpace, n, m, false));

    Material topoMat;
    topoMat.program = loadProgram("shaders/vs_basic.bin", "shaders/fs_basic.bin");
    topoMat.color[0] = 0.2;
    topoMat.color[1] = 0.2; 
    topoMat.color[2] = 1.0;
    topoMat.color[3] = 1.0;

    auto applyMat = [&](unsigned id)
    {
        GetRegistry().materialModel.insert(id, topoMat);
    };

    auto buildEntity = [&](unsigned id) -> Entity
    {
        Entity ent;
        ent.mesh_id = id;
        ent.material_id = id;
        ent.position = {0,0,0};

        ent.rotation = bx::Quaternion{0, 0, 0, 1.0f};
        ent.scale = {1.0f, 1.0f, 1.0f};
        mxSRT_Update(id, ent);
        GetRegistry().entities.insert(id, ent);
        return ent;
    };

    auto constructEntity = [&](unsigned id) -> Entity
    {
        Entity ent = buildEntity(id);
        applyMat(id);
        addEntity(ent);
        return ent;
    };

    constructEntity(id);*/
}

void SceneManager::updateTransforms() 
{
    HashTable<unsigned int, Entity>::PairData* array_entities = GetRegistry().entities.data();
    unsigned count = GetRegistry().entities.size();

    for (unsigned i = 0; i < count; i++)
        mxSRT_Update(array_entities[i].clave, array_entities[i].dato);
}

void SceneManager::mxSRT_Update(unsigned id, Entity& ent)
{
    ParametricSurface* surface = GetRegistry().parametricSurfaces.getPointer(id);
    bx::Vec3 c = {0, 0, 0}; ///< Centroid

    float model[16];
    float trans[16];
    float scale[16];
    float rot[16];
    float intermediate[16];

    bx::mtxScale(scale, ent.scale.x, ent.scale.y, ent.scale.z);
    bx::mtxFromQuaternion(rot, ent.rotation);

    if (surface != nullptr && GetRegistry().metricSpace != nullptr)
    {
        c = ent.position; 
        GetRegistry().metricSpace->computeCentroid();
        c = GetRegistry().metricSpace->getCentroid();
    }


    bx::mtxScale(scale, ent.scale.x, ent.scale.y, ent.scale.z);
    bx::mtxFromQuaternion(rot, ent.rotation);

    // Center the geometrical origin to world positions
    bx::mtxTranslate(trans, ent.position.x - c.x, ent.position.y - c.y, ent.position.z - c.z);
    
    bx::mtxMul(intermediate, scale, rot);
    bx::mtxMul(model, intermediate, trans);

    Material *mat = GetRegistry().materialModel.getPointer(ent.material_id);

    if (mat != nullptr)
        std::memcpy(mat->transform, model, sizeof(float) * 16);
}

bgfx::ShaderHandle SceneManager::loadShader(const char* path)
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

bgfx::ProgramHandle SceneManager::loadProgram(const char* vsPath, const char* fsPath)
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

void SceneManager::updateTopology()
{
    for (unsigned id : m_activeEntities)
    {
        Entity ent;
        ParametricSurface surface;

        if (GetRegistry().entities.search(id, ent) && 
            GetRegistry().parametricSurfaces.search(id, surface))
        {
            if (surface.needs_rebuild)
            {
                // 1. Translate engine's enum to the topological policy
                size_t enumIndex = static_cast<size_t>(surface.type);
                
                // Avoid buffer overflows
                assert(enumIndex < (sizeof(g_engineToTopologyMap) / sizeof(g_engineToTopologyMap[0])));

                const TopologyPolicies::WordSurfacePolicy& basePolicy = *g_engineToTopologyMap[enumIndex];

                // 2. Construct the policy
                TopologyPolicies::WordSurfacePolicy policy(
                    basePolicy.getWord(), 
                    surface.u_subdivisions, 
                    surface.v_subdivisions, 
                    basePolicy.getEmbedding()
                );
                
                static bx::DefaultAllocator defAlloc;
                std::shared_ptr<MetricSpace> metricSpace = std::make_shared<MetricSpace>(&defAlloc, policy);
                
                metricSpace->buildNorm(surface.p_norm);
                GetRegistry().metricSpace = metricSpace;
                // 3. Obtain the mesh
                Mesh mesh = Primitives::Surface(*metricSpace, surface.u_subdivisions, surface.v_subdivisions, surface.use_heatmap);

                /// Memory clean-up
                Mesh* old_mesh = GetRegistry().meshes.getPointer(ent.mesh_id);
                if (old_mesh != nullptr) 
                {
                    if (bgfx::isValid(old_mesh->vbh))
                        bgfx::destroy(old_mesh->vbh);

                    if (bgfx::isValid(old_mesh->ibh))
                        bgfx::destroy(old_mesh->ibh);
                    *old_mesh = mesh;
                }
                else
                    GetRegistry().meshes.insert(ent.mesh_id, mesh);

                // 4. Rengenerate surface
                GetRegistry().meshes.insert(ent.mesh_id, mesh);
                surface.needs_rebuild = false;
                GetRegistry().parametricSurfaces.insert(id, surface); 
                
                std::cout << "[DEBUG] Entity's topology " << id << " regenerated." << std::endl;
            }
        }
    }
}