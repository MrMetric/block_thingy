#include "RenderWorld.hpp"

#include <memory>
#include <utility>

#include <glm/trigonometric.hpp>

#include "Game.hpp"
#include "Gfx.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/default_view_frustum.hpp"
#include "graphics/null_frustum.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "physics/AABB.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

using std::shared_ptr;
using std::string;

using Graphics::OpenGL::ShaderProgram;
using Position::BlockInWorld;
using Position::ChunkInWorld;

std::tuple<uint64_t, uint64_t> RenderWorld::draw_world
(
	World& world,
	ResourceManager& resource_manager,
	const glm::dmat4& vp_matrix_,
	const Position::BlockInWorld& origin,
	const ChunkInWorld::value_type render_distance
)
{
	const glm::dvec3 camera_position = Game::instance->camera.position;
	const glm::dvec3 camera_rotation = Game::instance->camera.rotation;
	const ChunkInWorld camera_chunk{BlockInWorld(camera_position)};

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

	std::unique_ptr<Graphics::frustum> frustum;
	if(Settings::get<bool>("frustum_culling"))
	{
		const string projection_type = Settings::get<string>("projection_type");
		const glm::dvec3& pos = Gfx::instance->graphical_position;
		const glm::dvec3 rot = glm::radians(camera_rotation);
		const double near = Settings::get<double>("near_plane");
		const double far = Settings::get<double>("far_plane");
		const double fov = glm::radians(Settings::get<double>("fov"));
		const double ratio = static_cast<double>(Gfx::instance->window_size.x) / Gfx::instance->window_size.y;
		if(projection_type == "default")
		{
			frustum = std::make_unique<Graphics::default_view_frustum>(pos, rot, near, far, fov, ratio);
		}
		else if(projection_type == "infinite")
		{
			frustum = std::make_unique<Graphics::default_view_frustum>(pos, rot, near, fov, ratio);
		}
		else if(projection_type == "ortho")
		{
			// TODO
		}
		// note that no frustum is set if projection_type is an invalid value,
		// but the default projection type is used for rendering
	}
	if(frustum == nullptr)
	{
		frustum = std::make_unique<Graphics::null_frustum<true>>();
	}

	const ChunkInWorld chunk_pos(origin);
	const ChunkInWorld min = chunk_pos - render_distance;
	const ChunkInWorld max = chunk_pos + render_distance;

	const bool show_chunk_outlines = Settings::get<bool>("show_chunk_outlines");

	std::vector<std::shared_ptr<Chunk>> drawn_chunks;

	ChunkInWorld pos;
	for(pos.x = min.x; pos.x <= max.x; ++pos.x)
	for(pos.y = min.y; pos.y <= max.y; ++pos.y)
	for(pos.z = min.z; pos.z <= max.z; ++pos.z)
	{
		const ChunkInWorld gpos(pos - camera_chunk);
		const Physics::AABB aabb(gpos);
		if(!frustum->inside(aabb))
		{
			continue;
		}

		shared_ptr<Chunk> chunk = world.get_or_make_chunk(pos);

		if(show_chunk_outlines)
		{
			const glm::dvec3 min(static_cast<BlockInWorld::vec_type>(BlockInWorld(pos, {})));
			const glm::dvec3 max(min + static_cast<double>(CHUNK_SIZE));
			glm::dvec4 color(glm::uninitialize);
			if(chunk == nullptr)
			{
				// not loaded = red
				color = glm::dvec4(1, 0, 0, 1);
			}
			// TODO: add generating or loading = orange
			else if(world.is_meshing_queued(chunk))
			{
				// meshing = yellow
				color = glm::dvec4(1, 1, 0, 1);
			}
			else
			{
				// loaded = green
				color = glm::dvec4(0, 1, 0, 1);
			}
			Gfx::instance->draw_box_outline(min, max, color);
		}
		if(chunk != nullptr)
		{
			// false = opaque pass. Only opaque blocks will be drawn.
			chunk->render(false);
			drawn_chunks.emplace_back(chunk);
		}
	}

	for(auto& chunk : drawn_chunks)
	{
		// true = translucent pass. Only blocks with transparency will be drawn.
		chunk->render(true);
	}

	// TODO: render_distance should be unsigned
	uint64_t total = static_cast<uint64_t>(render_distance) * 2 + 1;
	total = total * total * total;
	const uint64_t drawn = static_cast<uint64_t>(drawn_chunks.size());
	return {total, drawn};
}
