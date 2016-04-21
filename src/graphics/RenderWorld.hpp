#pragma once

#include <map>

#include <glm/mat4x4.hpp>

#include "../BlockType.hpp"
#include "../position/ChunkInWorld.hpp"

class ShaderProgram;
class World;
namespace Position
{
	struct BlockInWorld;
}

namespace RenderWorld
{
	void draw_world(
		World& world,
		const std::map<BlockType, ShaderProgram>& block_shaders,
		const glm::dmat4& matriks,
		const Position::BlockInWorld& origin,
		ChunkInWorld_type render_distance
	);
}
