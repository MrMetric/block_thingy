#pragma once

#include <glm/vec3.hpp>

#include "fwd/position/BlockInWorld.hpp"

struct AABB // axis-aligned bounding box
{
	AABB();
	AABB(const Position::BlockInWorld&);

	glm::dvec3 min;
	glm::dvec3 max;

	bool collide(const AABB&);
	glm::dvec3 offset(const AABB&, const glm::vec3& direction_sign);
};
