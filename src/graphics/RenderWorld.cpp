#include "RenderWorld.hpp"

#include <memory>
#include <utility>

#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "position/ChunkInWorld.hpp"

using std::shared_ptr;

using Graphics::OpenGL::ShaderProgram;

void RenderWorld::draw_world(
	World& world,
	const std::map<BlockType, ShaderProgram>& block_shaders,
	const glm::dmat4& matriks,
	const Position::BlockInWorld& origin,
	const Position::ChunkInWorld::value_type render_distance
)
{
	for(const auto& p : block_shaders)
	{
		const ShaderProgram& shader = p.second;
		shader.uniform("matriks", glm::mat4(matriks));
	}

	Position::ChunkInWorld chunk_pos(origin);
	Position::ChunkInWorld min = chunk_pos - render_distance;
	Position::ChunkInWorld max = chunk_pos + render_distance;
	auto draw_chunks = [&world, &min, &max](const bool transluscent_pass)
	{
		for(Position::ChunkInWorld::value_type x = min.x; x <= max.x; ++x)
		{
			for(Position::ChunkInWorld::value_type y = min.y; y <= max.y; ++y)
			{
				for(Position::ChunkInWorld::value_type z = min.z; z <= max.z; ++z)
				{
					shared_ptr<Chunk> chunk = world.get_or_make_chunk({ x, y, z });
					chunk->render(transluscent_pass);
				}
			}
		}
	};
	draw_chunks(false);
	draw_chunks(true);
}
