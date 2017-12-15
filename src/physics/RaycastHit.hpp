#pragma once

#include <glm/vec3.hpp>

#include "fwd/block/Enum/Face.hpp"
#include "position/BlockInWorld.hpp"

namespace block_thingy::physics {

struct RaycastHit
{
	RaycastHit
	(
		const position::BlockInWorld& pos,
		const glm::ivec3& face
	);

	position::BlockInWorld pos;
	glm::ivec3 face_vec;

	position::BlockInWorld adjacent() const;
	block::enums::Face face() const;
};

}
