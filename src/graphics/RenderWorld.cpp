#include "RenderWorld.hpp"

#include <memory>
#include <utility>

#include "Gfx.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

using std::shared_ptr;

using Graphics::OpenGL::ShaderProgram;
using Position::BlockInWorld;
using Position::ChunkInWorld;

void RenderWorld::draw_world
(
	World& world,
	ResourceManager& resource_manager,
	const glm::dmat4& vp_matrix_,
	const Position::BlockInWorld& origin,
	const ChunkInWorld::value_type render_distance
)
{
	const glm::mat4 vp_matrix(vp_matrix_);
	resource_manager.foreach_ShaderProgram([&vp_matrix](Resource<Graphics::OpenGL::ShaderProgram> r)
	{
		#ifdef _WIN32
		if(Util::string_starts_with(r.get_id(), "shaders\\block\\"))
		#else
		if(Util::string_starts_with(r.get_id(), "shaders/block/"))
		#endif
		{
			r->uniform("mvp_matrix", vp_matrix);
		}
	});

	// TODO: frustrum culling

	const ChunkInWorld chunk_pos(origin);
	const ChunkInWorld min = chunk_pos - render_distance;
	const ChunkInWorld max = chunk_pos + render_distance;

	const bool show_chunk_outlines = Settings::get<bool>("show_chunk_outlines");
	// TODO: what if a chunk loads between passes?
	auto draw_chunks = [&world, &min, &max, show_chunk_outlines](const bool transluscent_pass)
	{
		ChunkInWorld pos;
		for(pos.x = min.x; pos.x <= max.x; ++pos.x)
		for(pos.y = min.y; pos.y <= max.y; ++pos.y)
		for(pos.z = min.z; pos.z <= max.z; ++pos.z)
		{
			shared_ptr<Chunk> chunk = world.get_or_make_chunk(pos);
			if(!transluscent_pass && show_chunk_outlines)
			{
				const glm::dvec3 min(static_cast<BlockInWorld::vec_type>(BlockInWorld(pos, {})));
				const glm::dvec3 max(min + static_cast<double>(CHUNK_SIZE));
				glm::dvec4 color;
				if(chunk == nullptr)
				{
					color = glm::dvec4(1, 0, 0, 1);
				}
				else if(world.is_meshing_queued(chunk))
				{
					color = glm::dvec4(1, 1, 0, 1);
				}
				else
				{
					color = glm::dvec4(0, 1, 0, 1);
				}
				Gfx::instance->draw_box_outline(min, max, color);
			}
			if(chunk != nullptr)
			{
				chunk->render(transluscent_pass);
			}
		}
	};
	draw_chunks(false);
	draw_chunks(true);
}
