#include "Chunk.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <graphics/OpenGL/ShaderProgram.hpp>
#include <graphics/OpenGL/VertexBuffer.hpp>

#include <Poco/BinaryReader.h>
using Poco::BinaryReader;
#include <Poco/BinaryWriter.h>
using Poco::BinaryWriter;

#include "mesh/ChunkMesher.hpp"
#include "mesh/GreedyMesher.hpp"
#include "../Block.hpp"
#include "../BlockType.hpp"
#include "../Game.hpp"
#include "../Gfx.hpp"
#include "../World.hpp"
#include "../position/BlockInChunk.hpp"
#include "../position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

Chunk::Chunk(const Position::ChunkInWorld& pos, World* owner)
	:
	owner(owner),
	position(pos),
	mesher(std::make_unique<GreedyMesher>(*this)),
	changed(true),
	solid_block(BlockType::air) // a useful default for now
{
}

Chunk::Chunk(BinaryReader& reader, World* owner)
	:
	owner(owner),
	mesher(std::make_unique<GreedyMesher>(*this)),
	changed(true)
{
	deserialize(reader);
}

inline static std::array<Block, CHUNK_BLOCK_COUNT>::size_type blok_index(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
{
	return CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;
}

const Block& Chunk::get_block_const(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z) const
{
	if(blok == nullptr)
	{
		return solid_block;
	}
	return (*blok)[blok_index(x, y, z)];
}

const Block& Chunk::get_block_const(const Position::BlockInChunk& pos) const
{
	if(blok == nullptr)
	{
		return solid_block;
	}
	return (*blok)[blok_index(pos.x, pos.y, pos.z)];
}

Block& Chunk::get_block_mutable(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
{
	init_blok();
	return (*blok)[blok_index(x, y, z)];
}

Block& Chunk::get_block_mutable(const Position::BlockInChunk& pos)
{
	init_blok();
	return (*blok)[blok_index(pos.x, pos.y, pos.z)];
}

void Chunk::set_block(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z, const Block& block)
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		std::string set_info = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") = " + std::to_string(block.type_id());
		throw std::domain_error("position out of bounds in Chunk::set: " + set_info);
	}

	init_blok();
	(*blok)[blok_index(x, y, z)] = block;
	changed = true;

	update_neighbors(x, y, z);
}

void Chunk::set_block(const Position::BlockInChunk& block_pos, const Block& block)
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
			mesh_vbos.emplace_back();
		}
	}

	size_t i = 0;
	for(auto p : meshes)
	{
		const mesh_t& mesh = p.second;
		mesh_vbos[i].data(mesh.size() * sizeof(mesh_t::value_type), mesh.data(), GL_DYNAMIC_DRAW);
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

	const Position::ChunkInWorld pos_mod = position * CHUNK_SIZE;
	size_t i = 0;
	for(auto p : meshes)
	{
		const BlockType type = p.first;
		const ShaderProgram& shader = Game::instance->gfx.get_block_shader(type);
		glUseProgram(shader.get_name());

		shader.uniform3f("pos_mod", pos_mod.x, pos_mod.y, pos_mod.z);
		shader.uniform1f("global_time", glfwGetTime());

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, mesh_vbos[i].get_name());
		glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		size_t draw_count = p.second.size() * 3;
		glDrawArrays(GL_TRIANGLES, 0, draw_count);
		glDisableVertexAttribArray(0);

		++i;
	}
}

void Chunk::init_blok()
{
	if(blok != nullptr)
	{
		return;
	}

	blok = std::make_unique<std::array<Block, CHUNK_BLOCK_COUNT>>();
	blok->fill(solid_block);
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

void Chunk::serialize(BinaryWriter& writer)
{
	writer << position.x << position.y << position.z;

	if(blok == nullptr)
	{
		writer << true;
		writer << solid_block.type_id();
	}
	else
	{
		writer << false;
		for(uint_fast32_t i = 0; i < blok->size(); ++i)
		{
			(*blok)[i].serialize(writer);
		}
	}
}

void Chunk::deserialize(BinaryReader& reader)
{
	reader >> position.x;
	reader >> position.y;
	reader >> position.z;

	bool is_solid;
	reader >> is_solid;
	if(is_solid)
	{
		blok = nullptr;
		block_type_id_t type_id;
		reader >> type_id;
		solid_block = Block(type_id);
	}
	else
	{
		blok = std::make_unique<std::array<Block, CHUNK_BLOCK_COUNT>>();
		for(uint_fast32_t i = 0; i < blok->size(); ++i)
		{
			(*blok)[i] = Block(reader);
		}
	}
}
