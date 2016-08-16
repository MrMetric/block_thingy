#pragma once

#include <glm/mat4x4.hpp>

#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include "types/window_size_t.hpp"

class Gfx;
class EventManager;

class GUI
{
	public:
		GUI(EventManager& event_manager);

		GUI(GUI&&) = delete;
		GUI(const GUI&) = delete;
		void operator=(const GUI&) = delete;

		void update_framebuffer_size(const window_size_t&);
		void draw(Gfx&);

	private:
		Graphics::OpenGL::ShaderProgram s_crosshair;
		Graphics::OpenGL::VertexBuffer crosshair_vbo;
		glm::dmat4 projection_matrix;

		void draw_crosshair();
};
