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
	float midX = Gfx::width / 2.0f;
	float midY = Gfx::height / 2.0f;
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_vertex), crosshair_vertex, GL_STATIC_DRAW);
}

void GUI::draw_crosshair()
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(Gfx::sp_crosshair);
	glUniformMatrix4fv(Gfx::vs_crosshair_matriks, 1, GL_FALSE, Gfx::matriks_ptr);

	glm::mat4 fuck = glm::ortho(0.0f, float(Gfx::width), float(Gfx::height), 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(Gfx::vs_crosshair_matriks, 1, GL_FALSE, glm::value_ptr(fuck));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->crosshair_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glDisableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
}