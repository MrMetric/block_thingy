#pragma once

#include <glm/vec3.hpp>

#include "block/block.hpp"
#include "fwd/block/enums/Face.hpp"
#include "position/block_in_world.hpp"

namespace block_thingy::physics {

struct raycast_hit
{
	raycast_hit
	(
		const position::block_in_world& pos,
		block_t,
		const glm::ivec3& face
	);

	position::block_in_world pos;
	block_t block;
	glm::ivec3 face_vec;

	position::block_in_world adjacent() const;
	block::enums::Face face() const;
};

}
