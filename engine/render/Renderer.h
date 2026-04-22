/******* Renderer ***************************************************/ /**
 *
 * @file Renderer.h
 *
 * Minimal renderer built over bgfx
 *  
 * @version 0
 * @author SpookyProDH-Coder
 * @date 15/01/2025
 ***************************************************************************/

#pragma once
#ifndef _RENDERER
#define _RENDERER

#include <bgfx/bgfx.h>
//#include <bgfx/platform.h>
//#include "glm/glm.hpp"

#include "../core/HashTable.h"

//typedef ID unsigned;

struct RenderCommand
{
	unsigned entity_id;
};

class Renderer
{
	public:
		Renderer();
		~Renderer();

		bool init(const bgfx::PlatformData&, unsigned, unsigned);
		void shutdown();
		void beginFrame();
		void submit(const RenderCommand&);
		void endFrame();

		void applyLighting(unsigned);

		void updateCamera(float, float, bool*);

	private:
		unsigned m_width = 0;
		unsigned m_height = 0;
		unsigned m_currentPreset = 0;

		bgfx::UniformHandle m_u_color;

		void setupView();

		bx::Vec3 m_camPos = {0.0f, 0.0f, -10.0f};
		float m_yaw = 0.0f;
		float m_pitch = 0.0f;

		// Sensibilidad
		float m_moveSpeed = 0.1f;
		float m_mouseSensitivity = 0.005f;
};

#endif