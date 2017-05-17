#include "Chunk.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include <glm/vec3.hpp>

#include "Camera.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/BlockRegistry.hpp"
#include "block/Enum/Type.hpp"
#include "chunk/Mesher/Base.hpp"
#include "graphics/Color.hpp"
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
		changed(false)
	{
	}

	World& owner;
	Position::ChunkInWorld position;

	ChunkData<Graphics::Color> light;

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

const Block::Base& Chunk::get_block(const BlockInChunk& pos) const
{
	return *blocks.get(pos);
}

Block::Base& Chunk::get_block(const BlockInChunk& pos)
{
	return *blocks.get(pos);
}

void Chunk::set_block(const BlockInChunk& pos, unique_ptr<Block::Base> block)
{
	blocks.set(pos, std::move(block));
}

Graphics::Color Chunk::get_light(const BlockInChunk& pos) const
{
	return pImpl->light.get(pos);
}

void Chunk::set_light(const BlockInChunk& pos, const Graphics::Color& color)
{
	pImpl->light.set(pos, color);
}

void Chunk::update()
{
	Mesher::meshmap_t meshes = pImpl->owner.mesher->make_mesh(*this);

	std::lock_guard<std::mutex> g(pImpl->mesh_mutex);
	pImpl->meshes = std::move(meshes);
	pImpl->changed = true;
}

void Chunk::render(const bool transluscent_pass)
{
	std::lock_guard<std::mutex> g(pImpl->mesh_mutex);

	if(pImpl->changed)
	{
		pImpl->update_vaos();
		pImpl->changed = false;
	}

	const ChunkInWorld render_position = pImpl->position - ChunkInWorld(BlockInWorld(Game::instance->camera.position));
	// TODO?: use double when available
	const glm::vec3 position_offset(static_cast<BlockInWorld::vec_type>(BlockInWorld(render_position, {0, 0, 0})));
	std::size_t i = 0;
	for(const auto& p : pImpl->meshes)
	{
		const BlockType type = p.first.block_type;
		// TODO: get existing block instead of making one
		if(pImpl->owner.block_registry.make(type)->is_translucent() != transluscent_pass)
		{
			++i;
			continue;
		}
		auto& shader = Game::instance->gfx.get_block_shader(type);
		glUseProgram(shader.get_name());

		shader.uniform("position_offset", position_offset);
		shader.uniform("tex", p.first.tex_unit);

		const std::size_t draw_count = p.second.size() * 3;
		pImpl->mesh_vaos[i].draw(GL_TRIANGLES, 0, draw_count);

		++i;
	}
}

void Chunk::set_blocks(chunk_blocks_t new_blocks)
{
	blocks = std::move(new_blocks);
}
void Chunk::set_blocks(unique_ptr<Block::Base> block)
{
	if(block == nullptr)
	{
		throw std::invalid_argument("Chunk::set_blocks(single): got a null block");
	}
	blocks.fill(std::move(block));
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
				{1, GL_UNSIGNED_BYTE}, // face
				{3, GL_FLOAT        }, // light1
				{3, GL_FLOAT        }, // light2
				{3, GL_FLOAT        }, // light3
				{3, GL_FLOAT        }, // light4
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
