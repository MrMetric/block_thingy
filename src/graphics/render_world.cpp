#include "render_world.hpp"

#include <memory>
#include <utility>

#include <glm/trigonometric.hpp>

#include "game.hpp"
#include "Gfx.hpp"
#include "resource_manager.hpp"
#include "settings.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/default_view_frustum.hpp"
#include "graphics/null_frustum.hpp"
#include "graphics/opengl/shader_program.hpp"
#include "physics/AABB.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"
#include "util/misc.hpp"
#include "world/world.hpp"

using std::shared_ptr;
using std::string;

namespace block_thingy::graphics {

using graphics::opengl::shader_program;
using position::block_in_world;
using position::chunk_in_world;

std::tuple<uint64_t, uint64_t> draw_world
(
	world::world& world,
	resource_manager& resource_manager,
	const glm::dmat4& vp_matrix_,
	const block_in_world& origin,
	const uint64_t render_distance
)
{
	const glm::dvec3 camera_position = game::instance->camera.position;
	const glm::dvec3 camera_rotation = game::instance->camera.rotation;
	const chunk_in_world camera_chunk{block_in_world(camera_position)};

	const glm::mat4 vp_matrix(vp_matrix_);
	resource_manager.foreach_shader_program([&vp_matrix](resource<shader_program> r)
	{
	#ifdef _WIN32
		if(util::string_starts_with(r.get_id(), "shaders\\block\\"))
	#else
		if(util::string_starts_with(r.get_id(), "shaders/block/"))
	#endif
		{
			r->uniform("mvp_matrix", vp_matrix);
		}
	});

	std::unique_ptr<frustum> frustum_;
	if(settings::get<bool>("frustum_culling"))
	{
		const string projection_type = settings::get<string>("projection_type");
		const glm::dvec3& pos = Gfx::instance->graphical_position;
		const glm::dvec3 rot = glm::radians(camera_rotation);
		const double near = settings::get<double>("near_plane");
		const double far = settings::get<double>("far_plane");
		const double fov = glm::radians(settings::get<double>("fov"));
		const double ratio = static_cast<double>(Gfx::instance->window_size.x) / Gfx::instance->window_size.y;
		if(projection_type == "default")
		{
			frustum_ = std::make_unique<default_view_frustum>(pos, rot, near, far, fov, ratio);
		}
		else if(projection_type == "infinite")
		{
			frustum_ = std::make_unique<default_view_frustum>(pos, rot, near, fov, ratio);
		}
		else if(projection_type == "ortho")
		{
			// TODO
		}
		// note that no frustum is set if projection_type is an invalid value,
		// but the default projection type is used for rendering
	}
	if(frustum_ == nullptr)
	{
		frustum_ = std::make_unique<null_frustum<true>>();
	}

	const chunk_in_world chunk_pos(origin);
	const chunk_in_world::value_type render_distance_ = static_cast<chunk_in_world::value_type>(render_distance);
	const chunk_in_world min = chunk_pos - render_distance_;
	const chunk_in_world max = chunk_pos + render_distance_;

	const bool show_chunk_outlines = settings::get<bool>("show_chunk_outlines");

	std::vector<shared_ptr<Chunk>> drawn_chunks;

	chunk_in_world pos;
	for(pos.x = min.x; pos.x <= max.x; ++pos.x)
	for(pos.y = min.y; pos.y <= max.y; ++pos.y)
	for(pos.z = min.z; pos.z <= max.z; ++pos.z)
	{
		const chunk_in_world gpos(pos - camera_chunk);
		const physics::AABB aabb(gpos);
		if(!frustum_->inside(aabb))
		{
			continue;
		}

		shared_ptr<Chunk> chunk = world.get_or_make_chunk(pos);

		if(show_chunk_outlines)
		{
			const glm::dvec3 min(static_cast<block_in_world::vec_type>(block_in_world(pos, {})));
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

	uint64_t total = render_distance * 2 + 1;
	total = total * total * total;
	const uint64_t drawn = static_cast<uint64_t>(drawn_chunks.size());
	return {total, drawn};
}

}
