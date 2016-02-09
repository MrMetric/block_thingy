#include "Chunk.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include <glad/glad.h>

#include "mesh/ChunkMesher.hpp"
#include "mesh/SimpleMesher.hpp"
#include "../Block.hpp"
#include "../Coords.hpp"
#include "../World.hpp"

Chunk::Chunk(Position::ChunkInWorld pos, World* owner)
	:
	owner(owner),
	position(pos),
	mesher(std::make_unique<SimpleMesher>(*this)),
	changed(true)
{
	glGenBuffers(1, &mesh_vbo);
}

Chunk::~Chunk()
{
	glDeleteBuffers(1, &mesh_vbo);
}

Block Chunk::get_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const
{
	return blok[CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x];
}

Block Chunk::get_block(Position::BlockInChunk bcp) const
{
	return blok[CHUNK_SIZE * CHUNK_SIZE * bcp.y + CHUNK_SIZE * bcp.z + bcp.x];
}

void Chunk::set(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, Block block)
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		std::string set_info = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") = " + std::to_string(block.type());
		throw std::domain_error("position out of bounds in Chunk::set: " + set_info);
	}

	uint_fast32_t index = CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;
	blok[index] = block;
	changed = true;

	update_neighbors(x, y, z);
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
	vertexes = mesher->make_mesh();
	draw_count = static_cast<GLsizei>(vertexes.size());
	#ifdef COOL_DEBUG_STUFF
	if(draw_count % 3 != 0)
	{
		throw std::logic_error("you buggered something up again!");
	}
	#endif
	draw_count /= 3;

	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(GLbyte), vertexes.data(), GL_DYNAMIC_DRAW);

	changed = false;
}

void Chunk::render()
{
	if(changed)
	{
		update();
	}

	glUniform3f(owner->vs_cube_pos_mod, position.x * CHUNK_SIZE, position.y * CHUNK_SIZE, position.z * CHUNK_SIZE);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, draw_count);
	glDisableVertexAttribArray(0);

	//glUniform3f(owner->vs_cube_pos_mod, 0, 0, 0);
}

void Chunk::update_neighbors(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z)
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

void Chunk::update_neighbor(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z)
{
	Position::ChunkInWorld cp(x, y, z);
	cp += position;
	std::shared_ptr<Chunk> chunk = owner->get_chunk(cp);
	if(chunk != nullptr)
	{
		chunk->changed = true;
	}
}
