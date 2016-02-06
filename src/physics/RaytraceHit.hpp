#pragma once

#include <glm/vec3.hpp>

#include "../Coords.hpp"

struct RaytraceHit
{
	RaytraceHit(Position::BlockInWorld pos, glm::ivec3 face);

	Position::BlockInWorld pos;
	glm::ivec3 face;

	Position::BlockInWorld adjacent();
};
