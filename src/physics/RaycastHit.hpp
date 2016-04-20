#pragma once

#include <glm/vec3.hpp>

#include "../position/BlockInWorld.hpp"

struct RaycastHit
{
	RaycastHit(const Position::BlockInWorld& pos, const glm::ivec3& face);

	Position::BlockInWorld pos;
	glm::ivec3 face;

	Position::BlockInWorld adjacent();
};
