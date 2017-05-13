#include "RenderWorld.hpp"

#include <memory>
#include <utility>

#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "position/ChunkInWorld.hpp"

using std::shared_ptr;

using Graphics::OpenGL::ShaderProgram;
using Position::ChunkInWorld;

void RenderWorld::draw_world
(
	World& world,
	std::map<BlockType, ShaderProgram>& block_shaders,
	const glm::dmat4& matriks,
	const Position::BlockInWorld& origin,
	const ChunkInWorld::value_type render_distance
)
{
	for(auto& p : block_shaders)
	{
		ShaderProgram& shader = p.second;
		shader.uniform("matriks", glm::mat4(matriks));
	}

	// TODO: frustrum culling

	const ChunkInWorld chunk_pos(origin);
	const ChunkInWorld min = chunk_pos - render_distance;
	const ChunkInWorld max = chunk_pos + render_distance;

	// TODO: what if a chunk loads between passes?
	auto draw_chunks = [&world, &min, &max](const bool transluscent_pass)
	{
		for(ChunkInWorld::value_type x = min.x; x <= max.x; ++x)
		for(ChunkInWorld::value_type y = min.y; y <= max.y; ++y)
		for(ChunkInWorld::value_type z = min.z; z <= max.z; ++z)
		{
			shared_ptr<Chunk> chunk = world.get_or_make_chunk({ x, y, z });
			if(chunk != nullptr)
			{
				chunk->render(transluscent_pass);
			}
		}
	};
	draw_chunks(false);
	draw_chunks(true);
}
