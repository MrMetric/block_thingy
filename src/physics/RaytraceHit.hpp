#pragma once

#include <glm/vec3.hpp>

#include "../position/BlockInWorld.hpp"

struct RaytraceHit
{
	RaytraceHit(const Position::BlockInWorld& pos, glm::ivec3 face);

	Position::BlockInWorld pos;
	glm::ivec3 face;

	Position::BlockInWorld adjacent();
};
