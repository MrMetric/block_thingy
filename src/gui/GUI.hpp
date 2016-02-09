#pragma once

#include <glad/glad.h>

class Gfx;

class GUI
{
	public:
		GUI();
		virtual ~GUI();

		void update_framebuffer_size(uint_fast32_t width, uint_fast32_t height);
		void draw(const Gfx&);

	private:
		GLuint crosshair_vbo;

		void draw_crosshair(const Gfx&);
};
