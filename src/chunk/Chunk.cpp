#include "Chunk.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include <glm/vec3.hpp>

#include "Camera.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "World.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

Chunk::Chunk(const Position::ChunkInWorld& pos, World& owner)
	:
	owner(owner),
	position(pos),
	changed(true),
	solid_block(BlockType::air) // a useful default for now
{
}

inline static chunk_block_array_t::size_type block_array_index(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
{
	return CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;
}

const Block::Block& Chunk::get_block_const(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z) const
{
	if(blocks == nullptr)
	{
		return solid_block;
	}
	return blocks->at(block_array_index(x, y, z));
}

const Block::Block& Chunk::get_block_const(const Position::BlockInChunk& pos) const
{
	if(blocks == nullptr)
	{
		return solid_block;
	}
	return blocks->at(block_array_index(pos.x, pos.y, pos.z));
}

Block::Block& Chunk::get_block_mutable(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
{
	init_block_array();
	return blocks->at(block_array_index(x, y, z));
}

Block::Block& Chunk::get_block_mutable(const Position::BlockInChunk& pos)
{
	init_block_array();
	return blocks->at(block_array_index(pos.x, pos.y, pos.z));
}

void Chunk::set_block(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z, const Block::Block& block)
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		std::string set_info = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") = " + std::to_string(block.type_id());
		throw std::domain_error("position out of bounds in Chunk::set: " + set_info);
	}

	init_block_array();
	blocks->at(block_array_index(x, y, z)) = block;
	changed = true;

	update_neighbors(x, y, z);
}

void Chunk::set_block(const Position::BlockInChunk& block_pos, const Block::Block& block)
{
	set_block(block_pos.x, block_pos.y, block_pos.z, block);
}

Position::ChunkInWorld Chunk::get_position() const
{
	return position;
}

World& Chunk::get_owner() const
{
	return owner;
}

void Chunk::update()
{
	if(blocks == nullptr && solid_block.is_invisible())
	{
		meshes = meshmap_t();
		return;
	}

	meshes = owner.mesher->make_mesh(*this);

	if(mesh_vbos.size() < meshes.size())
	{
		size_t to_add = meshes.size() - mesh_vbos.size();
		for(size_t i = 0; i < to_add; ++i)
		{
			mesh_vbos.emplace_back();
		}
	}

	size_t i = 0;
	for(const auto& p : meshes)
	{
		const mesh_t& mesh = p.second;
		mesh_vbos[i].data(mesh.size() * sizeof(mesh_t::value_type), mesh.data(), VertexBuffer::UsageHint::dynamic_draw);
		++i;
	}
}

void Chunk::render(const bool transluscent_pass)
{
	if(changed)
	{
		update();
		changed = false;
	}

	const Position::ChunkInWorld render_position = position - Position::ChunkInWorld(Position::BlockInWorld(Game::instance->camera.position));
	const Position::BlockInWorld position_render_offset(render_position, {0, 0, 0});
	size_t i = 0;
	for(const auto& p : meshes)
	{
		const BlockType type = p.first;
		if(Block::Block(type).is_translucent() != transluscent_pass)
		{
			++i;
			continue;
		}
		const ShaderProgram& shader = Game::instance->gfx.get_block_shader(type);
		glUseProgram(shader.get_name());

		shader.uniform("position_offset", static_cast<glm::vec3>(position_render_offset));
		shader.uniform("global_time", static_cast<float>(glfwGetTime())); // TODO: use double when available

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, mesh_vbos[i].get_name());
		glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		size_t draw_count = p.second.size() * 3;
		glDrawArrays(GL_TRIANGLES, 0, draw_count);
		glDisableVertexAttribArray(0);

		++i;
	}
}

void Chunk::init_block_array()
{
	if(blocks != nullptr)
	{
		return;
	}

	blocks = std::make_unique<chunk_block_array_t>();
	blocks->fill(solid_block);
}

void Chunk::update_neighbors(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
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

void Chunk::update_neighbor(const ChunkInWorld_type x, const ChunkInWorld_type y, const ChunkInWorld_type z)
{
	Position::ChunkInWorld chunk_pos(x, y, z);
	chunk_pos += position;
	std::shared_ptr<Chunk> chunk = owner.get_chunk(chunk_pos);
	if(chunk != nullptr)
	{
		chunk->changed = true;
	}
}
