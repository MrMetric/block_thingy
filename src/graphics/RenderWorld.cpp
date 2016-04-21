#include "RenderWorld.hpp"

#include <memory>
#include <utility>

#include "OpenGL/ShaderProgram.hpp"
#include "../World.hpp"
#include "../chunk/Chunk.hpp"
#include "../position/BlockInWorld.hpp"
#include "../position/ChunkInWorld.hpp"

void RenderWorld::draw_world(
	World& world,
	const std::map<BlockType, ShaderProgram>& block_shaders,
	const glm::mat4& matriks,
	const Position::BlockInWorld& origin,
	const ChunkInWorld_type render_distance
)
{
	for(const auto& p : block_shaders)
	{
		const ShaderProgram& shader = p.second;
		shader.uniform("matriks", matriks);
	}

	Position::ChunkInWorld chunk_pos(origin);
	Position::ChunkInWorld min = chunk_pos - render_distance;
	Position::ChunkInWorld max = chunk_pos + render_distance;
	for(ChunkInWorld_type x = min.x; x <= max.x; ++x)
	{
		for(ChunkInWorld_type y = min.y; y <= max.y; ++y)
		{
			for(ChunkInWorld_type z = min.z; z <= max.z; ++z)
			{
				std::shared_ptr<Chunk> chunk = world.get_or_make_chunk({ x, y, z });
				chunk->render();
			}
		}
	}
}
