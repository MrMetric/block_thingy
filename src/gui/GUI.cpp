#include "GUI.hpp"

#include <glm/gtx/transform.hpp> // glm::ortho
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include "../Game.hpp"
#include "../Gfx.hpp"

GUI::GUI()
{
	glGenBuffers(1, &this->crosshair_vbo);
}

GUI::~GUI()
{
	glDeleteBuffers(1, &this->crosshair_vbo);
}

void GUI::update_framebuffer_size()
{
	float midX = Game::instance->gfx.width / 2.0f;
	float midY = Game::instance->gfx.height / 2.0f;
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
	glBindBuffer(GL_ARRAY_BUFFER, this->crosshair_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_vertex), crosshair_vertex, GL_DYNAMIC_DRAW);
}

void GUI::draw()
{
	this->draw_crosshair();
}

void GUI::draw_crosshair()
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(Game::instance->gfx.sp_crosshair);
	glUniformMatrix4fv(Game::instance->gfx.vs_crosshair_matriks, 1, GL_FALSE, Game::instance->gfx.matriks_ptr);

	glm::mat4 crosshair_matrix = glm::ortho(0.0f, float(Game::instance->gfx.width), float(Game::instance->gfx.height), 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(Game::instance->gfx.vs_crosshair_matriks, 1, GL_FALSE, glm::value_ptr(crosshair_matrix));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->crosshair_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glDisableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
}
