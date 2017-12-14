#pragma once

#include <map>
#include <tuple>

#include <glm/mat4x4.hpp>

#include "fwd/ResourceManager.hpp"
#include "fwd/World.hpp"
#include "fwd/block/Enum/Type.hpp"
#include "fwd/graphics/OpenGL/ShaderProgram.hpp"
#include "fwd/position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

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
	position::ChunkInWorld::value_type render_distance
);

}
