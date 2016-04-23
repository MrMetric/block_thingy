#pragma once

#include <cstdint>

#include <graphics/OpenGL/ShaderProgram.hpp>
#include <graphics/OpenGL/VertexBuffer.hpp>

#include <types/window_size_t.hpp>

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
		void draw(const Gfx&);

	private:
		ShaderProgram s_crosshair;
		VertexBuffer crosshair_vbo;

		void draw_crosshair(const Gfx&);
};
