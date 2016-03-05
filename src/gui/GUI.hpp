#pragma once

#include <cstdint>

#include <graphics/OpenGL/Shader.hpp>
#include <graphics/OpenGL/VertexBuffer.hpp>

class Gfx;
class EventManager;

class GUI
{
	public:
		GUI(EventManager& event_manager);

		void update_framebuffer_size(uint_fast32_t width, uint_fast32_t height);
		void draw(const Gfx&);

	private:
		Shader s_crosshair;
		VertexBuffer crosshair_vbo;

		void draw_crosshair(const Gfx&);
};
