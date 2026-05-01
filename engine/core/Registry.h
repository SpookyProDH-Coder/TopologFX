#ifndef _REGISTRY_H
#define _REGISTRY_H

#include "HashTable.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>

struct Mesh
{
	bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
};

struct Material
{
	float color[4];
	float transform[16];

	bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle u_color;
	bgfx::UniformHandle u_model;
	bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
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

struct LightingPreset
{
	unsigned clearColor;
	float intensity;
};

struct PhysicsModel
{
	float mass;
	float friction;
};

struct Registry
{
	unsigned size = 64;
	HashTable<unsigned, Mesh> meshes{size};
	HashTable<unsigned, Material> materialModel{size};
	HashTable<unsigned, Entity> entities{size};
	HashTable<unsigned, LightingPreset> lightingPreset{size};
	HashTable<unsigned, PhysicsModel> physicsModel{64};
};

// Explicit access point
Registry& GetRegistry();

#endif