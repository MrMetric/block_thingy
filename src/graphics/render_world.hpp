#pragma once

#include <tuple>

#include <glm/mat4x4.hpp>

#include "fwd/resource_manager.hpp"
#include "fwd/position/block_in_world.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy::graphics {

/**
 * @return The amount of chunks considered for drawing and the amount of chunks drawn
 */
std::tuple<uint64_t, uint64_t> draw_world
(
	world::world&,
	resource_manager&,
	const glm::dmat4& vp_matrix,
	const position::block_in_world& origin,
	uint64_t render_distance
);

}
