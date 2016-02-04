#include "Chunk.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "mesh/ChunkMesher.hpp"
#include "mesh/SimpleMesher.hpp"
#include "../Block.hpp"
#include "../Coords.hpp"
#include "../World.hpp"

Chunk::Chunk(Position::ChunkInWorld pos, World* owner)
	:
	owner(owner),
	position(pos),
	mesher(std::make_unique<SimpleMesher>(*this))
{
	this->init();
}

Chunk::~Chunk()
{
	glDeleteBuffers(1, &this->mesh_vbo);
}

void Chunk::init()
{
	this->changed = true;

	glGenBuffers(1, &this->mesh_vbo);
}

Block Chunk::get_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const
{
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x];
}

Block Chunk::get_block(Position::BlockInChunk bcp) const
{
	return this->blok[CHUNK_SIZE * CHUNK_SIZE * bcp.y + CHUNK_SIZE * bcp.z + bcp.x];
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
	this->blok[index] = block;
	this->changed = true;

	this->update_neighbors(x, y, z);
}

Position::ChunkInWorld Chunk::get_position() const
{
	return this->position;
}

World* Chunk::get_owner() const
{
	return this->owner;
}

/** TODO: figure out what causes the weird chunk rendering error
INFO:
	it happens on both of my laptop computers (Intel graphics cards, GNU/Linux (3.3) and Windows (3.0))
	zel says it does not happen with his AMD card (Windows (up to 4.4))
	zel notes: 'there is a fine difference between fixing a bug and breaking app so the bug does not appear'
*/
void Chunk::update()
{
	#ifdef WEIRD_BUG_FIX
	auto old_size = this->vertexes.size();
	#endif

	this->vertexes = this->mesher->make_mesh();
	this->draw_count = static_cast<GLsizei>(this->vertexes.size());

	#ifdef WEIRD_BUG_FIX
	if(this->vertexes.size() < old_size)
	{
		this->vertexes.reserve(old_size);
		while(this->vertexes.size() < old_size)
		{
			this->vertexes.push_back(0);
		}
	}
	#endif

	glBindBuffer(GL_ARRAY_BUFFER, this->mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertexes.size() * sizeof(GLfloat), this->vertexes.data(), GL_DYNAMIC_DRAW);

	this->changed = false;
}

GLenum Chunk::render_mode = GL_TRIANGLES;

void Chunk::render()
{
	if(this->changed)
	{
		this->update();
	}

	glUniform3f(this->owner->vs_cube_pos_mod, this->position.x * CHUNK_SIZE, this->position.y * CHUNK_SIZE, this->position.z * CHUNK_SIZE);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->mesh_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(Chunk::render_mode, 0, this->draw_count);
	glDisableVertexAttribArray(0);

	//glUniform3f(this->owner->vs_cube_pos_mod, 0, 0, 0);
}

void Chunk::update_neighbors(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z)
{
	// TODO: check if the neighbor chunk has a block beside this one (to avoid updating when the appearance won't change)
	if(x == 0)
	{
		this->update_neighbor(-1, 0, 0);
	}
	else if(x == CHUNK_SIZE - 1)
	{
		this->update_neighbor(+1, 0, 0);
	}
	if(y == 0)
	{
		this->update_neighbor(0, -1, 0);
	}
	else if(y == CHUNK_SIZE - 1)
	{
		this->update_neighbor(0, +1, 0);
	}
	if(z == 0)
	{
		this->update_neighbor(0, 0, -1);
	}
	else if(z == CHUNK_SIZE - 1)
	{
		this->update_neighbor(0, 0, +1);
	}
}

void Chunk::update_neighbor(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z)
{
	Position::ChunkInWorld cp(x + this->position.x, y + this->position.y, z + this->position.z);
	std::shared_ptr<Chunk> chunk = this->owner->get_chunk(cp);
	if(chunk != nullptr)
	{
		chunk->changed = true;
	}
}
