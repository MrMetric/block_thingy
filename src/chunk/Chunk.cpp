#include "Chunk.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <stdint.h>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "game.hpp"
#include "settings.hpp"
#include "chunk/Mesher/base.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_change_setting.hpp"
#include "graphics/camera.hpp"
#include "graphics/opengl/shader_program.hpp"
#include "graphics/opengl/texture.hpp"
#include "graphics/opengl/vertex_array.hpp"
#include "graphics/opengl/vertex_buffer.hpp"
#include "position/block_in_chunk.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"
#include "util/logger.hpp"
#include "world/world.hpp"

using std::nullopt;
using std::unique_ptr;

namespace block_thingy {

using position::block_in_chunk;
using position::block_in_world;
using position::chunk_in_world;

constexpr uint32_t CHUNK_SIZE_2 = CHUNK_SIZE + 2;

struct Chunk::impl
{
	impl
	(
		const chunk_in_world& position,
		world::world& owner
	)
	:
		owner(owner),
		position(position),
		light_changed(false),
		changed(false)
	{
		light_tex_buf.fill(0);

		light_smoothing_eid = game::instance->event_manager.add_handler(EventType::change_setting, [this](const Event& event)
		{
			const auto& e = static_cast<const Event_change_setting&>(event);

			if(light_tex == nullptr)
			{
				return;
			}

			if(e.name == "light_smoothing")
			{
				const int64_t light_smoothing = *e.new_value.get<int64_t>();
				const GLint mag_filter = static_cast<GLint>((light_smoothing == 0) ? GL_NEAREST : GL_LINEAR);
				light_tex->parameter(graphics::opengl::texture::Parameter::mag_filter, mag_filter);
			}
		});
	}

	~impl()
	{
		try
		{
			game::instance->event_manager.unadd_handler(light_smoothing_eid);
		}
		catch(const std::runtime_error& e)
		{
			// should never happen, but it is best to be safe (destructors are noexcept)
			LOG(BUG) << "error unadding event handler in Chunk::impl destructor: " << e.what() << '\n';
		}
	}

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	void init_light_tex()
	{
		light_tex = std::make_unique<graphics::opengl::texture>(GL_TEXTURE_3D);
		set_light_tex_data();
		light_tex->parameter(graphics::opengl::texture::Parameter::wrap_s, GL_CLAMP_TO_EDGE);
		light_tex->parameter(graphics::opengl::texture::Parameter::wrap_t, GL_CLAMP_TO_EDGE);
		light_tex->parameter(graphics::opengl::texture::Parameter::min_filter, GL_NEAREST);
		const GLint mag_filter = static_cast<GLint>((settings::get<int64_t>("light_smoothing") == 0) ? GL_NEAREST : GL_LINEAR);
		light_tex->parameter(graphics::opengl::texture::Parameter::mag_filter, mag_filter);
		light_changed = false;
	}

	void set_light_tex_data()
	{
		light_tex->image3D(0, GL_RGB, CHUNK_SIZE_2, CHUNK_SIZE_2, CHUNK_SIZE_2, GL_RGB, GL_UNSIGNED_BYTE, light_tex_buf.data());
	}

	void set_texbuflight(const glm::ivec3& pos, const graphics::color& color);

	world::world& owner;
	chunk_in_world position;

	unique_ptr<graphics::opengl::texture> light_tex;
	bool light_changed;
	event_handler_id_t light_smoothing_eid;

	bool changed;
	mesher::meshmap_t meshes;
	std::vector<graphics::opengl::vertex_array> mesh_vaos;
	std::vector<graphics::opengl::vertex_buffer> mesh_vbos;
	mutable std::mutex mesh_mutex;

	void update_vaos();

private:
	std::array<uint8_t, CHUNK_SIZE_2 * CHUNK_SIZE_2 * CHUNK_SIZE_2 * 3> light_tex_buf;
};

Chunk::Chunk(const chunk_in_world& position, world::world& owner)
:
	blocks([&owner]() -> block_t
	{
		const auto air = owner.block_manager.get_block("air");
		if(air != nullopt)
		{
			return *air;
		}
		return {};
	}()),
	pImpl(std::make_unique<impl>(position, owner))
{
}

Chunk::~Chunk()
{
}

world::world& Chunk::get_owner() const
{
	return pImpl->owner;
}

chunk_in_world Chunk::get_position() const
{
	return pImpl->position;
}

block_t Chunk::get_block(const block_in_chunk& pos) const
{
	return blocks.get(pos);
}

void Chunk::set_block(const block_in_chunk& pos, block_t block)
{
	blocks.set(pos, block);
}

graphics::color Chunk::get_light(const block_in_chunk& pos) const
{
	const graphics::color light1 = get_blocklight(pos);
	const graphics::color light2 = get_skylight(pos);
	return graphics::color
	{
		std::max(light1.r, light2.r),
		std::max(light1.g, light2.g),
		std::max(light1.b, light2.b),
	};
}

graphics::color Chunk::get_blocklight(const block_in_chunk& pos) const
{
	return blocklight.get(pos);
}

void Chunk::set_blocklight
(
	const block_in_chunk& pos,
	const graphics::color& color
)
{
	blocklight.set(pos, color);
	set_texbuflight({pos.x, pos.y, pos.z}, get_light(pos));
}

graphics::color Chunk::get_skylight(const block_in_chunk& pos) const
{
	return skylight.get(pos);
}

void Chunk::set_skylight
(
	const block_in_chunk& pos,
	const graphics::color& color
)
{
	skylight.set(pos, color);
	set_texbuflight({pos.x, pos.y, pos.z}, get_light(pos));
}

void Chunk::set_texbuflight(const glm::ivec3& pos, const graphics::color& color)
{
	pImpl->set_texbuflight(pos, color);
}
void Chunk::impl::set_texbuflight(const glm::ivec3& pos, const graphics::color& color)
{
	const std::size_t i = 3 * (
			  static_cast<std::size_t>(pos.z + 1) * CHUNK_SIZE_2 * CHUNK_SIZE_2
			+ static_cast<std::size_t>(pos.y + 1) * CHUNK_SIZE_2
			+ static_cast<std::size_t>(pos.x + 1)
		);
	light_tex_buf[i    ] = color.r;
	light_tex_buf[i + 1] = color.g;
	light_tex_buf[i + 2] = color.b;
	light_changed = true;
}

void Chunk::update()
{
	mesher::meshmap_t meshes = pImpl->owner.mesher->make_mesh(*this);

	std::lock_guard<std::mutex> g(pImpl->mesh_mutex);
	pImpl->meshes = std::move(meshes);
	pImpl->changed = true;
}

void Chunk::render(const bool translucent_pass)
{
	std::lock_guard<std::mutex> g(pImpl->mesh_mutex);

	if(pImpl->meshes.empty())
	{
		return;
	}

	if(pImpl->changed)
	{
		if(pImpl->light_tex == nullptr)
		{
			pImpl->init_light_tex();
		}

		pImpl->update_vaos();

		pImpl->changed = false;
	}

	if(pImpl->light_changed && pImpl->light_tex != nullptr)
	{
		pImpl->set_light_tex_data();
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(pImpl->light_tex->type, pImpl->light_tex->get_name());

	const chunk_in_world render_position = pImpl->position - chunk_in_world(block_in_world(game::instance->camera.position));
	// TODO?: use double when available
	const glm::vec3 position_offset(static_cast<block_in_world::vec_type>(block_in_world(render_position, {0, 0, 0})));
	std::size_t i = 0;
	for(const auto& p : pImpl->meshes)
	{
		if(p.first.is_translucent != translucent_pass)
		{
			++i;
			continue;
		}

		resource<graphics::opengl::shader_program> shader = game::instance->resource_manager.get_shader_program(p.first.shader_path);
		shader->uniform("position_offset", position_offset);
		shader->uniform("tex", p.first.tex_unit);

		shader->use();
		pImpl->mesh_vaos[i].draw(GL_TRIANGLES, 0, p.second.size() * 3);

		++i;
	}
}

void Chunk::regenerate_texbuflight()
{
	block_in_chunk pos;
	for(pos.x = 0; pos.x < CHUNK_SIZE; ++pos.x)
	for(pos.y = 0; pos.y < CHUNK_SIZE; ++pos.y)
	for(pos.z = 0; pos.z < CHUNK_SIZE; ++pos.z)
	{
		set_texbuflight({pos.x, pos.y, pos.z}, get_light(pos));
	}
}

void Chunk::impl::update_vaos()
{
	if(mesh_vaos.size() < meshes.size())
	{
		const std::size_t to_add = meshes.size() - mesh_vaos.size();
		for(std::size_t i = 0; i < to_add; ++i)
		{
			graphics::opengl::vertex_buffer vbo
			({
				{3, GL_UNSIGNED_BYTE}, // relative position
				{1, GL_UNSIGNED_BYTE}, // face (3 bits) and rotation (2 bits)
				{1, GL_SHORT        }, // texture index
			});
			graphics::opengl::vertex_array vao(vbo);

			mesh_vbos.emplace_back(std::move(vbo));
			mesh_vaos.emplace_back(std::move(vao));
		}
	}
	// TODO: delete unused

	std::size_t i = 0;
	for(const auto& p : meshes)
	{
		const auto usage_hint = graphics::opengl::vertex_buffer::usage_hint::dynamic_draw;
		const mesher::mesh_t& mesh = p.second;
		mesh_vbos[i].data(mesh.size() * sizeof(mesher::mesh_t::value_type), mesh.data(), usage_hint);
		++i;
	}
}

}
