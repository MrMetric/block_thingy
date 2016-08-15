#pragma once

#include <map>

#include <glm/mat4x4.hpp>

#include "fwd/World.hpp"
#include "fwd/block/BlockType.hpp"
#include "fwd/position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

class ShaderProgram;

namespace RenderWorld
{
	void draw_world(
		World& world,
		const std::map<BlockType, ShaderProgram>& block_shaders,
		const glm::dmat4& matriks,
		const Position::BlockInWorld& origin,
		Position::ChunkInWorld::value_type render_distance
	);
}
