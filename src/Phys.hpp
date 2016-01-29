#pragma once

#include <glm/mat4x4.hpp>
#include <GL/glew.h>

namespace Position
{
	struct BlockInWorld;
}

class Phys
{
	public:
		Phys();
		virtual ~Phys();

		void step(const glm::mat4& perspective_matrix, const glm::mat4& view_matrix);

		double gravity;

		Position::BlockInWorld* bwp;
		glm::ivec3 face;

		void draw_outline(int64_t x, int64_t y, int64_t z, const glm::vec4& color = glm::vec4(1, 1, 1, 1));

	private:
		GLuint outline_vbo;

		void raycast(glm::dvec3 origin, glm::dvec3 direction, double radius);
};