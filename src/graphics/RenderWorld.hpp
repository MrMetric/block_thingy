#pragma once

#include <map>

#include <glm/mat4x4.hpp>

#include "fwd/World.hpp"
#include "fwd/block/BlockType.hpp"
#include "fwd/graphics/OpenGL/ShaderProgram.hpp"
#include "fwd/position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

namespace RenderWorld
{
	void draw_world
	(
		World& world,
		std::map<BlockType, Graphics::OpenGL::ShaderProgram>& block_shaders,
		const glm::dmat4& matriks,
		const Position::BlockInWorld& origin,
		Position::ChunkInWorld::value_type render_distance
	);
}
