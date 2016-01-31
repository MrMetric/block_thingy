#pragma once

#include "../Coords.hpp"

struct RaytraceHit
{
	Position::BlockInWorld pos;
	glm::ivec3 face;

	Position::BlockInWorld adjacent();
};