#pragma once

#include <stdint.h>

#include <glad/glad.h>

class Cube
{
	public:
		static GLubyte cube_vertex[8 * 3];
		static uint_fast8_t cube_elements[12 * 3];
};
