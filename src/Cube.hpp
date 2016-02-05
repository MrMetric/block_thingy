#pragma once

#include <cstdint>

#include <GL/glew.h>

class Cube
{
	public:
		static GLbyte cube_vertex[8 * 3];
		static uint_fast8_t cube_elements[12 * 3];
};