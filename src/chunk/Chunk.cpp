#include "Chunk.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <glad/glad.h>
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include <glm/vec3.hpp>

#include "Camera.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/BlockRegistry.hpp"
#include "block/BlockType.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::to_string;
using std::shared_ptr;
using std::unique_ptr;

using Position::BlockInChunk;
using Position::BlockInWorld;
using Position::ChunkInWorld;

Chunk::Chunk(const ChunkInWorld& pos, World& owner)
:
	owner(owner),
	position(pos)
{
	set_blocks(owner.game.block_registry.make(BlockType::air));
}

World& Chunk::get_owner() const
{
	return owner;
}

ChunkInWorld Chunk::get_position() const
{
	return position;
}

inline static chunk_block_array_t::size_type block_array_index(const BlockInChunk::value_type x, const BlockInChunk::value_type y, const BlockInChunk::value_type z)
{
	return CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;
}

const Block::Base& Chunk::get_block(const BlockInChunk::value_type x, const BlockInChunk::value_type y, const BlockInChunk::value_type z) const
{
	return *blocks[block_array_index(x, y, z)];
}

const Block::Base& Chunk::get_block(const BlockInChunk& pos) const
{
	return *blocks[block_array_index(pos.x, pos.y, pos.z)];
}

Block::Base& Chunk::get_block_m(const BlockInChunk& pos)
{
	return *blocks[block_array_index(pos.x, pos.y, pos.z)];
}

void Chunk::set_block(const BlockInChunk::value_type x, const BlockInChunk::value_type y, const BlockInChunk::value_type z, unique_ptr<Block::Base> block)
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		string set_info = "(" + to_string(x) + ", " + to_string(y) + ", " + to_string(z) + ") = " + to_string(block->type_id());
		throw std::domain_error("position out of bounds in Chunk::set: " + set_info);
	}

	solid_block = nullptr;
	blocks[block_array_index(x, y, z)] = std::move(block);
	changed = true;

	update_neighbors(x, y, z);
}

void Chunk::set_block(const BlockInChunk& block_pos, unique_ptr<Block::Base> block)
{
	set_block(block_pos.x, block_pos.y, block_pos.z, std::move(block));
}

const Graphics::Color& Chunk::get_light(const BlockInChunk& pos) const
{
	const auto i = block_array_index(pos.x, pos.y, pos.z);
	return light[i];
}

void Chunk::set_light(const BlockInChunk& pos, const Graphics::Color& color)
{
	const auto i = block_array_index(pos.x, pos.y, pos.z);
	light[i] = color;
	changed = true;

	update_neighbors(pos.x, pos.y, pos.z);
}

void Chunk::update()
{
	if(solid_block != nullptr && solid_block->is_invisible())
	{
		meshes.clear();
		return;
	}

	meshes = owner.mesher->make_mesh(*this);
	update_vaos();
}

void Chunk::render(const bool transluscent_pass)
{
	if(changed)
	{
		update();
		changed = false;
	}

	const ChunkInWorld render_position = position - ChunkInWorld(BlockInWorld(owner.game.camera.position));
	const BlockInWorld position_render_offset(render_position, {0, 0, 0});
	std::size_t i = 0;
	for(const auto& p : meshes)
	{
		const Mesher::meshmap_key_t& key = p.first;
		const BlockType type = std::get<0>(key);
		if(owner.game.block_registry.make(type)->is_translucent() != transluscent_pass)
		{
			++i;
			continue;
		}
		auto& shader = owner.game.gfx.get_block_shader(type);
		glUseProgram(shader.get_name());

		shader.uniform("position_offset", static_cast<glm::vec3>(position_render_offset));
		shader.uniform("global_time", static_cast<float>(owner.get_time())); // TODO: use double when available

		const Graphics::Color color = std::get<1>(key);
		shader.uniform("light", static_cast<glm::vec3>(color));

		const std::size_t draw_count = p.second.size() * 3;
		mesh_vaos[i].draw(GL_TRIANGLES, 0, draw_count);

		++i;
	}
}

const Mesher::meshmap_t& Chunk::get_meshes() const
{
	return meshes;
}
void Chunk::set_meshes(const Mesher::meshmap_t& m)
{
	changed = false;
	meshes = m;
	update_vaos();
}

void Chunk::set_blocks(chunk_block_array_t new_blocks)
{
	blocks = std::move(new_blocks);
	solid_block = nullptr;
	changed = true;
}
void Chunk::set_blocks(unique_ptr<Block::Base> block)
{
	// TODO: compare new block with current block
	solid_block = std::move(block);
	std::generate(blocks.begin(), blocks.end(), [this]()
	{
		return owner.game.block_registry.make(*solid_block);
	});
	changed = true;
}

void Chunk::update_vaos()
{
	if(mesh_vaos.size() < meshes.size())
	{
		const std::size_t to_add = meshes.size() - mesh_vaos.size();
		for(std::size_t i = 0; i < to_add; ++i)
		{
			Graphics::OpenGL::VertexBuffer vbo({3, GL_UNSIGNED_BYTE});
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

void Chunk::update_neighbors() const
{
	update_neighbor(-1,  0,  0);
	update_neighbor(+1,  0,  0);
	update_neighbor( 0, -1,  0);
	update_neighbor( 0, +1,  0);
	update_neighbor( 0,  0, -1);
	update_neighbor( 0,  0, +1);
}

void Chunk::update_neighbors(const BlockInChunk::value_type x, const BlockInChunk::value_type y, const BlockInChunk::value_type z) const
{
	// TODO: check if the neighbor chunk has a block beside this one (to avoid updating when the appearance won't change)
	if(x == 0)
	{
		update_neighbor(-1, 0, 0);
	}
	else if(x == CHUNK_SIZE - 1)
	{
		update_neighbor(+1, 0, 0);
	}

	if(y == 0)
	{
		update_neighbor(0, -1, 0);
	}
	else if(y == CHUNK_SIZE - 1)
	{
		update_neighbor(0, +1, 0);
	}

	if(z == 0)
	{
		update_neighbor(0, 0, -1);
	}
	else if(z == CHUNK_SIZE - 1)
	{
		update_neighbor(0, 0, +1);
	}
}

void Chunk::update_neighbor(const ChunkInWorld::value_type x, const ChunkInWorld::value_type y, const ChunkInWorld::value_type z) const
{
	ChunkInWorld chunk_pos(x, y, z);
	chunk_pos += position;
	shared_ptr<Chunk> chunk = owner.get_chunk(chunk_pos);
	if(chunk != nullptr)
	{
		chunk->changed = true;
	}
}
