#pragma once

#include <GL/glew.h>

class GUI
{
	public:
		GUI();
		virtual ~GUI();

		void update_framebuffer_size();
		void draw();

	private:
		GLuint crosshair_vbo;

		void draw_crosshair();
};
