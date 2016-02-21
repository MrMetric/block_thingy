#include "Chunk.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include <glad/glad.h>

#include "mesh/ChunkMesher.hpp"
#include "mesh/GreedyMesher.hpp"
#include "../Block.hpp"
#include "../BlockType.hpp"
#include "../Coords.hpp"
#include "../Game.hpp"
#include "../Gfx.hpp"
#include "../World.hpp"
#include "../graphics/BlockShader.hpp"

Chunk::Chunk(Position::ChunkInWorld pos, World* owner)
	:
	owner(owner),
	position(pos),
	mesher(std::make_unique<GreedyMesher>(*this)),
	changed(true)
{
	// a useful default for now
	blok.fill(Block(BlockType::air));
}

Chunk::~Chunk()
{
	glDeleteBuffers(mesh_vbos.size(), &mesh_vbos[0]);
}

Block Chunk::get_block(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z) const
{
	return blok[CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x];
}

Block Chunk::get_block(const Position::BlockInChunk& block_pos) const
{
	return get_block(block_pos.x, block_pos.y, block_pos.z);
}

void Chunk::set_block(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z, Block block)
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		std::string set_info = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") = " + std::to_string(block.type_id());
		throw std::domain_error("position out of bounds in Chunk::set: " + set_info);
	}

	uint_fast32_t index = CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;
	blok[index] = block;
	changed = true;

	update_neighbors(x, y, z);
}

void Chunk::set_block(const Position::BlockInChunk& block_pos, Block block)
{
	set_block(block_pos.x, block_pos.y, block_pos.z, block);
}

Position::ChunkInWorld Chunk::get_position() const
{
	return position;
}

World* Chunk::get_owner() const
{
	return owner;
}

void Chunk::update()
{
	meshes = mesher->make_mesh();

	if(mesh_vbos.size() < meshes.size())
	{
		size_t to_add = meshes.size() - mesh_vbos.size();
		for(size_t i = 0; i < to_add; ++i)
		{
			GLuint vbo;
			glGenBuffers(1, &vbo);
			mesh_vbos.push_back(vbo);
		}
	}

	size_t i = 0;
	for(auto p : meshes)
	{
		const mesh_t& mesh = p.second;
		glBindBuffer(GL_ARRAY_BUFFER, mesh_vbos[i]);
		glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(mesh_t::value_type), mesh.data(), GL_DYNAMIC_DRAW);
		++i;
	}

	changed = false;
}

void Chunk::render()
{
	if(changed)
	{
		update();
	}

	size_t i = 0;
	for(auto p : meshes)
	{
		const BlockType type = p.first;
		const BlockShader& shader = Game::instance->gfx.get_block_shader(type);
		glUseProgram(shader.program);
		glEnableVertexAttribArray(0);
		Position::ChunkInWorld pos_mod = position * CHUNK_SIZE;
		shader.uniform3f("pos_mod", pos_mod.x, pos_mod.y, pos_mod.z);
		glBindBuffer(GL_ARRAY_BUFFER, mesh_vbos[i]);
		glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);
		size_t draw_count = p.second.size() * 3;
		glDrawArrays(GL_TRIANGLES, 0, draw_count);
		glDisableVertexAttribArray(0);
		++i;
	}
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
	std::shared_ptr<Chunk> chunk = owner->get_chunk(chunk_pos);
	if(chunk != nullptr)
	{
		chunk->changed = true;
	}
}
