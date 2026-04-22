#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <iostream>

#include "Renderer.h"
#include "../core/HashTable.h"
#include "../core/Registry.h"

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
		this->m_u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);

		if (!bgfx::isValid(m_u_color))
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
	bgfx::shutdown();
}

void Renderer::beginFrame()
{
	bgfx::touch(0);
}

void Renderer::submit(const RenderCommand& cmd)
{
	assert(m_width > 0 && m_height > 0);

	Registry& r = GetRegistry();
	Entity ent;
	Mesh m;
	Material mat;

	if(!r.entities.search(cmd.entity_id, ent))
	{
		cerr << "[!] Error: Couldn't draw the full entity:\n" << " - Entity id: " << cmd.entity_id << endl;
		return;
	}
	if(!r.meshes.search(ent.mesh_id, m))
	{
		cerr << "[!] Error: Couldn't draw the full entity:\n" << " - Mesh id: " << ent.mesh_id << endl;
		return;
	}

	if(!r.materialModel.search(ent.material_id, mat))
	{
		cerr << "[!] Error: Couldn't draw the full entity:\n" << " - Material id: " << ent.material_id << endl;
		return;
	}
		
	
	if (!bgfx::isValid(m.vbh) || !bgfx::isValid(m.ibh) || !bgfx::isValid(mat.program)) 
	{
		cerr << "[!] Invalid handles for entity submission:" << endl;
		if (!bgfx::isValid(m.vbh)) cerr << "  - Vertex Buffer is INVALID" << endl;
		if (!bgfx::isValid(m.ibh)) cerr << "  - Index Buffer is INVALID" << endl;
		if (!bgfx::isValid(mat.program)) cerr << "  - Shader Program is INVALID" << endl;
		return;
	}

	bgfx::setTransform(ent.transform);
	bgfx::setVertexBuffer(0, m.vbh);
	bgfx::setIndexBuffer(m.ibh);
	bgfx::setUniform(m_u_color, &mat.color);
	bgfx::setState(BGFX_STATE_DEFAULT);

	/*uint64_t state = BGFX_STATE_WRITE_RGB 
                   | BGFX_STATE_WRITE_A 
				   | BGFX_STATE_WRITE_Z
				   | BGFX_STATE_DEPTH_TEST_LESS
                   | BGFX_STATE_PT_LINES;
	bgfx::setState(state);*/
	bgfx::submit(0, mat.program);
}

void Renderer::endFrame()
{
	bgfx::frame();
}

void Renderer::applyLighting(unsigned presetID)
{
	Registry& r = GetRegistry();

	LightingPreset preset{};

	if (!r.lightingPreset.search(presetID, preset))
		cerr << "[!] Preset " << presetID << " not found!" << endl;
	else
	{
		m_currentPreset = presetID;

		bgfx::setViewClear(
			0,
			BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
			preset.clearColor,
			1.0f,
			0
		);
	}
	return;
}

void Renderer::setupView()
{
	assert(m_width > 0 && m_height > 0);
	bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

	const bx::Vec3 at  = { 0.0f, 0.0f,  0.0f };
    const bx::Vec3 eye = { 0.0f, 0.0f, -5.0f };

	float view[16];
	bx::mtxLookAt(view, eye, at);

	float proj[16];
	bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

	bgfx::setViewTransform(0, view, proj);
}

void Renderer::updateCamera(float _dx, float _dy, bool _keys[6])
{
    // 1. Actualizar rotación con el movimiento del ratón
    m_yaw += _dx * m_mouseSensitivity;
    m_pitch -= _dy * m_mouseSensitivity; // Invertido para que sea natural

    // Limitar el pitch para no dar la vuelta completa (gimbal lock)
    m_pitch = bx::clamp(m_pitch, -bx::kPiHalf + 0.1f, bx::kPiHalf - 0.1f);

    // 2. Calcular vector Forward (donde miramos)
    bx::Vec3 forward = {
        bx::cos(m_pitch) * bx::sin(m_yaw),
        bx::sin(m_pitch),
        bx::cos(m_pitch) * bx::cos(m_yaw)
    };

    // 3. Calcular vectores Right y Up
    bx::Vec3 up = { 0.0f, 1.0f, 0.0f };
    bx::Vec3 right = bx::normalize(bx::cross(up, forward));
    
    // 4. Mover posición según teclado (W:0, S:1, A:2, D:3, Q:4, E:5)
    if (_keys[0]) m_camPos = bx::add(m_camPos, bx::mul(forward, m_moveSpeed)); // W
    if (_keys[1]) m_camPos = bx::sub(m_camPos, bx::mul(forward, m_moveSpeed)); // S
    if (_keys[2]) m_camPos = bx::add(m_camPos, bx::mul(right, m_moveSpeed));   // A
    if (_keys[3]) m_camPos = bx::sub(m_camPos, bx::mul(right, m_moveSpeed));   // D

    // 5. Re-calcular la matriz de vista
    float view[16];
    bx::Vec3 at = bx::add(m_camPos, forward); // El punto al que miramos
    bx::mtxLookAt(view, m_camPos, at, up);

    // 6. Proyección (puedes mantener la de tu setupView actual)
    float proj[16];
    bx::mtxProj(proj, 60.0f, float(m_width)/float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, proj);
}