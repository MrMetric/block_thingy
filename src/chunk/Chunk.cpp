#include "Chunk.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Camera.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "Settings.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/BlockRegistry.hpp"
#include "block/Enum/Type.hpp"
#include "chunk/Mesher/Base.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_change_setting.hpp"
#include "graphics/Color.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

using std::string;
using std::to_string;
using std::shared_ptr;
using std::unique_ptr;

using Position::BlockInChunk;
using Position::BlockInWorld;
using Position::ChunkInWorld;

constexpr int_fast32_t CHUNK_SIZE_2 = CHUNK_SIZE + 2;

struct Chunk::impl
{
	impl
	(
		const ChunkInWorld& position,
		World& owner
	)
	:
		owner(owner),
		position(position),
		light_changed(false),
		changed(false)
	{
		light_tex_buf.fill(0);

		light_smoothing_eid = Game::instance->event_manager.add_handler(EventType::change_setting, [this](const Event& event)
		{
			auto e = static_cast<const Event_change_setting&>(event);

			if(light_tex == nullptr)
			{
				return;
			}

			if(e.name == "light_smoothing")
			{
				const int64_t light_smoothing = *e.value.get<int64_t>();
				const GLenum mag_filter = (light_smoothing == 0) ? GL_NEAREST : GL_LINEAR;
				light_tex->parameter(Graphics::OpenGL::Texture::Parameter::mag_filter, mag_filter);
			}
		});
	}

	~impl()
	{
		Game::instance->event_manager.unadd_handler(light_smoothing_eid);
	}

	void init_light_tex()
	{
		light_tex = std::make_unique<Graphics::OpenGL::Texture>(GL_TEXTURE_3D);
		set_light_tex_data();
		light_tex->parameter(Graphics::OpenGL::Texture::Parameter::wrap_s, GL_CLAMP_TO_EDGE);
		light_tex->parameter(Graphics::OpenGL::Texture::Parameter::wrap_t, GL_CLAMP_TO_EDGE);
		light_tex->parameter(Graphics::OpenGL::Texture::Parameter::min_filter, GL_NEAREST);
		const GLenum mag_filter = (Settings::get<int64_t>("light_smoothing") == 0) ? GL_NEAREST : GL_LINEAR;
		light_tex->parameter(Graphics::OpenGL::Texture::Parameter::mag_filter, mag_filter);
		light_changed = false;
	}

	void set_light_tex_data()
	{
		light_tex->image3D(0, GL_RGB, CHUNK_SIZE_2, CHUNK_SIZE_2, CHUNK_SIZE_2, GL_RGB, GL_UNSIGNED_BYTE, light_tex_buf.data());
	}

	World& owner;
	Position::ChunkInWorld position;

	ChunkData<Graphics::Color> light;
	std::unique_ptr<Graphics::OpenGL::Texture> light_tex;
	std::array<uint8_t, CHUNK_SIZE_2 * CHUNK_SIZE_2 * CHUNK_SIZE_2 * 3> light_tex_buf;
	bool light_changed;
	event_handler_id_t light_smoothing_eid;

	bool changed;
	Mesher::meshmap_t meshes;
	std::vector<Graphics::OpenGL::VertexArray> mesh_vaos;
	std::vector<Graphics::OpenGL::VertexBuffer> mesh_vbos;
	mutable std::mutex mesh_mutex;

	void update_vaos();
};

Chunk::Chunk(const ChunkInWorld& position, World& owner)
:
	pImpl(std::make_unique<impl>(position, owner))
{
}

Chunk::~Chunk()
{
}

World& Chunk::get_owner() const
{
	return pImpl->owner;
}

ChunkInWorld Chunk::get_position() const
{
	return pImpl->position;
}

const shared_ptr<Block::Base> Chunk::get_block(const BlockInChunk& pos) const
{
	return blocks.get(pos);
}

shared_ptr<Block::Base> Chunk::get_block(const BlockInChunk& pos)
{
	return blocks.get(pos);
}

void Chunk::set_block(const BlockInChunk& pos, shared_ptr<Block::Base> block)
{
	if(block == nullptr)
	{
		throw std::invalid_argument("Chunk::set_block: got a null block");
	}
	blocks.set(pos, block);
}

Graphics::Color Chunk::get_light(const BlockInChunk& pos) const
{
	return pImpl->light.get(pos);
}

void Chunk::set_light(const BlockInChunk& pos, const Graphics::Color& color)
{
	pImpl->light.set(pos, color);
	set_neighbor_light({pos.x, pos.y, pos.z}, color);
}

void Chunk::set_neighbor_light(const glm::ivec3& pos, const Graphics::Color& color)
{
	const size_t i = (CHUNK_SIZE_2 * CHUNK_SIZE_2 * (pos.z + 1) + CHUNK_SIZE_2 * (pos.y + 1) + pos.x + 1) * 3;
	pImpl->light_tex_buf[i    ] = std::round(color.r * 255.0 / Graphics::Color::max);
	pImpl->light_tex_buf[i + 1] = std::round(color.g * 255.0 / Graphics::Color::max);
	pImpl->light_tex_buf[i + 2] = std::round(color.b * 255.0 / Graphics::Color::max);

	pImpl->light_changed = true;
}

void Chunk::update()
{
	Mesher::meshmap_t meshes = pImpl->owner.mesher->make_mesh(*this);

	std::lock_guard<std::mutex> g(pImpl->mesh_mutex);
	pImpl->meshes = std::move(meshes);
	pImpl->changed = true;
}

void Chunk::render(const bool translucent_pass)
{
	std::lock_guard<std::mutex> g(pImpl->mesh_mutex);

	if(pImpl->changed)
	{
		if(pImpl->light_tex == nullptr)
		{
			pImpl->init_light_tex();
		}

		pImpl->update_vaos();
		pImpl->changed = false;
	}

	if(pImpl->meshes.empty())
	{
		return;
	}

	if(pImpl->light_changed && pImpl->light_tex != nullptr)
	{
		pImpl->set_light_tex_data();
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(pImpl->light_tex->type, pImpl->light_tex->get_name());

	const ChunkInWorld render_position = pImpl->position - ChunkInWorld(BlockInWorld(Game::instance->camera.position));
	// TODO?: use double when available
	const glm::vec3 position_offset(static_cast<BlockInWorld::vec_type>(BlockInWorld(render_position, {0, 0, 0})));
	std::size_t i = 0;
	for(const auto& p : pImpl->meshes)
	{
		if(p.first.is_translucent != translucent_pass)
		{
			++i;
			continue;
		}

		Resource<Graphics::OpenGL::ShaderProgram> shader = Game::instance->resource_manager.get_ShaderProgram(p.first.shader_path);
		shader->uniform("position_offset", position_offset);
		shader->uniform("tex", p.first.tex_unit);

		shader->use();
		pImpl->mesh_vaos[i].draw(GL_TRIANGLES, 0, p.second.size() * 3);

		++i;
	}
}

void Chunk::set_blocks(chunk_blocks_t new_blocks)
{
	blocks = std::move(new_blocks);
}
void Chunk::set_blocks(shared_ptr<Block::Base> block)
{
	if(block == nullptr)
	{
		throw std::invalid_argument("Chunk::set_blocks(single): got a null block");
	}
	blocks.fill(block);
}

void Chunk::impl::update_vaos()
{
	if(mesh_vaos.size() < meshes.size())
	{
		const std::size_t to_add = meshes.size() - mesh_vaos.size();
		for(std::size_t i = 0; i < to_add; ++i)
		{
			Graphics::OpenGL::VertexBuffer vbo
			({
				{3, GL_UNSIGNED_BYTE}, // relative position
				{1, GL_UNSIGNED_BYTE}, // face (3 bits) and rotation (2 bits)
				{1, GL_SHORT        }, // texture index
			});
			Graphics::OpenGL::VertexArray vao(vbo);

			mesh_vbos.emplace_back(std::move(vbo));
			mesh_vaos.emplace_back(std::move(vao));
		}
	}
	// TODO: delete unused

	std::size_t i = 0;
	for(const auto& p : meshes)
	{
		const auto usage_hint = Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw;
		const Mesher::mesh_t& mesh = p.second;
		mesh_vbos[i].data(mesh.size() * sizeof(Mesher::mesh_t::value_type), mesh.data(), usage_hint);
		++i;
	}
}
