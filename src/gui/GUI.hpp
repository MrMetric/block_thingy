#pragma once

#include <cstdint>

#include <graphics/OpenGL/VertexBuffer.hpp>

class Gfx;

class GUI
{
	public:
		GUI();

		void update_framebuffer_size(uint_fast32_t width, uint_fast32_t height);
		void draw(const Gfx&);

	private:
		VertexBuffer crosshair_vbo;

		void draw_crosshair(const Gfx&);
};
