#include "RenderWorld.hpp"

#include <memory>
#include <utility>

#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "position/ChunkInWorld.hpp"

using std::shared_ptr;

using Graphics::OpenGL::ShaderProgram;

void RenderWorld::draw_world
(
	World& world,
	std::map<BlockType, ShaderProgram>& block_shaders,
	const glm::dmat4& matriks,
	const Position::BlockInWorld& origin,
	const Position::ChunkInWorld::value_type render_distance
)
{
	for(auto& p : block_shaders)
	{
		ShaderProgram& shader = p.second;
		shader.uniform("matriks", glm::mat4(matriks));
	}

	// TODO: frustrum culling

	const Position::ChunkInWorld chunk_pos(origin);
	const Position::ChunkInWorld min = chunk_pos - render_distance;
	const Position::ChunkInWorld max = chunk_pos + render_distance;

	// TODO (when threading is added): what if a chunk loads between passes?
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
