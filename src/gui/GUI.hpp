#pragma once

#include <GL/glew.h>

class GUI
{
	public:
		GUI();
		virtual ~GUI();

		void update_framebuffer_size();
		void draw_crosshair();

	private:
		GLuint crosshair_vbo;

		char pad[4];
};