#pragma once

#include <glm/vec3.hpp>

#include "fwd/position/BlockInWorld.hpp"
#include "fwd/position/ChunkInWorld.hpp"

namespace block_thingy::physics {

struct AABB // axis-aligned bounding box
{
	AABB();
	AABB(const glm::dvec3& min, const glm::dvec3& max);
	AABB(const position::block_in_world&);
	AABB(const position::chunk_in_world&);

	glm::dvec3 min;
	glm::dvec3 max;

	bool collide(const AABB&) const;
	glm::dvec3 offset(const AABB&, const glm::vec3& direction_sign) const;
};

}
