#include "GUI.hpp"

#include <glad/glad.h>
#include <graphics/OpenGL/ShaderProgram.hpp>
#include <graphics/OpenGL/VertexBuffer.hpp>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho

#include "../Gfx.hpp"
#include "../event/EventManager.hpp"
#include "../event/EventType.hpp"
#include "../event/type/Event_window_size_change.hpp"

GUI::GUI(EventManager& event_manager)
	:
	s_crosshair("shaders/crosshair")
{
	event_manager.add_handler(EventType::window_size_change, [self=this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);
		self->update_framebuffer_size(e.width, e.height);
	});
}

void GUI::update_framebuffer_size(const uint_fast32_t width, const uint_fast32_t height)
{
	float midX = width / 2.0f;
	float midY = height / 2.0f;
	GLfloat crosshair_vertex[] = {
		midX - 16, midY - 1, 0,
		midX - 16, midY + 1, 0,
		midX + 16, midY + 1, 0,
		midX - 16, midY - 1, 0,
		midX + 16, midY + 1, 0,
		midX + 16, midY - 1, 0,

		midX - 1, midY - 16, 0,
		midX - 1, midY + 16, 0,
		midX + 1, midY + 16, 0,
		midX - 1, midY - 16, 0,
		midX + 1, midY + 16, 0,
		midX + 1, midY - 16, 0,
	};
	crosshair_vbo.data(sizeof(crosshair_vertex), crosshair_vertex, GL_DYNAMIC_DRAW);
}

void GUI::draw(const Gfx& gfx)
{
	draw_crosshair(gfx);
}

void GUI::draw_crosshair(const Gfx& gfx)
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(s_crosshair.get_name());
	s_crosshair.uniformMatrix4fv("matriks", gfx.matriks);

	glm::mat4 crosshair_matrix = glm::ortho(0.0f, static_cast<float>(gfx.width), static_cast<float>(gfx.height), 0.0f, -1.0f, 1.0f);
	s_crosshair.uniformMatrix4fv("matriks", crosshair_matrix);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo.get_name());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glDisableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
}
