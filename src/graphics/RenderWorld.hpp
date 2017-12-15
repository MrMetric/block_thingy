#pragma once

#include <tuple>

#include <glm/mat4x4.hpp>

#include "fwd/ResourceManager.hpp"
#include "fwd/World.hpp"
#include "fwd/position/BlockInWorld.hpp"

namespace block_thingy::graphics {

/**
 * @return The amount of chunks considered for drawing and the amount of chunks drawn
 */
std::tuple<uint64_t, uint64_t> draw_world
(
	World&,
	ResourceManager&,
	const glm::dmat4& vp_matrix,
	const position::BlockInWorld& origin,
	uint64_t render_distance
);

}
