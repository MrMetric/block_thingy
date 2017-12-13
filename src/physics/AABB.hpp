#pragma once

#include <glm/vec3.hpp>

#include "fwd/position/BlockInWorld.hpp"
#include "fwd/position/ChunkInWorld.hpp"

namespace Physics {

struct AABB // axis-aligned bounding box
{
	AABB();
	AABB(const glm::dvec3& min, const glm::dvec3& max);
	AABB(const Position::BlockInWorld&);
	AABB(const Position::ChunkInWorld&);

	glm::dvec3 min;
	glm::dvec3 max;

	bool collide(const AABB&);
	glm::dvec3 offset(const AABB&, const glm::vec3& direction_sign);
};

}
