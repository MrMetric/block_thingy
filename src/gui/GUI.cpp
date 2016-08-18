#include "GUI.hpp"

#include <glad/glad.h>
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho

#include "Game.hpp"
#include "Gfx.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"

GUI::GUI(EventManager& event_manager)
	:
	s_crosshair("shaders/crosshair"),
	crosshair_vbo({2, GL_FLOAT}),
	crosshair_vao(crosshair_vbo)
{
	event_manager.add_handler(EventType::window_size_change, [self=this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);
		self->update_framebuffer_size(e.window_size);
	});
}

void GUI::update_framebuffer_size(const window_size_t& window_size)
{
	float midX = window_size.x / 2.0f;
	float midY = window_size.y / 2.0f;
	float crosshair_vertex[] = {
		midX - 16, midY - 1,
		midX - 16, midY + 1,
		midX + 16, midY + 1,
		midX - 16, midY - 1,
		midX + 16, midY + 1,
		midX + 16, midY - 1,

		midX - 1, midY - 16,
		midX - 1, midY + 16,
		midX + 1, midY + 16,
		midX - 1, midY - 16,
		midX + 1, midY + 16,
		midX + 1, midY - 16,
	};
	const auto usage_hint = Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw;
	crosshair_vbo.data(sizeof(crosshair_vertex), crosshair_vertex, usage_hint);

	const double width = window_size.x;
	const double height = window_size.y;
	projection_matrix = glm::ortho(0.0, width, height, 0.0, -1.0, 1.0);
	s_crosshair.uniform("matriks", glm::mat4(projection_matrix));
}

void GUI::draw()
{
	draw_crosshair();
}

void GUI::draw_crosshair()
{
	const bool wireframe = Game::instance->wireframe();
	if(wireframe)
	{
		Game::instance->wireframe = false;
	}

	glDisable(GL_DEPTH_TEST);

	glUseProgram(s_crosshair.get_name());
	crosshair_vao.draw(GL_TRIANGLES, 0, 12);

	glEnable(GL_DEPTH_TEST);

	if(wireframe)
	{
		Game::instance->wireframe = true;
	}
}
