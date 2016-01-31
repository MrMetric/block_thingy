#include "World.hpp"

#include <GL/glew.h>

#include "Block.hpp"
#include "Chunk.hpp"
#include "Coords.hpp"
#include "Game.hpp"
#include "Gfx.hpp"

World::World()
	:
	last_chunk(nullptr)
{
}

World::~World()
{
	for(auto thingy : this->chunks)
	{
		delete thingy.second;
	}
}

uint64_t World::chunk_key(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z)
{
	static_assert(sizeof(ChunkInWorld_type) <= 4, "update chunk_key for new ChunkInWorld_type size");
	uint32_t x_ = x & 0xFFFFF;
	uint32_t y_ = y & 0xFFFFF;
	uint32_t z_ = z & 0xFFFFF;
	uint64_t key =	  (uint64_t(x_) << 40)
					| (uint64_t(y_) << 20)
					| (uint64_t(z_))
				;
	return key;
}

void World::set_block(Position::BlockInWorld bwp, Block* block, bool delete_old_block)
{
	Position::ChunkInWorld cp(bwp);
	Chunk* chunk = this->get_or_make_chunk(cp);

	Position::BlockInChunk bcp(bwp);
	chunk->set(bcp.x, bcp.y, bcp.z, block, delete_old_block);
}

Block* World::get_block(Position::BlockInWorld bwp) const
{
	Position::ChunkInWorld cp(bwp);
	Chunk* chunk = this->get_chunk(cp);
	if(chunk == nullptr)
	{
		return nullptr;
	}

	Position::BlockInChunk bcp(bwp);
	return chunk->get_block(bcp);
}

void World::set_chunk(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z, Chunk* chunk)
{
	uint64_t key = chunk_key(x, y, z);
	// TODO: delete old Chunk if overwriting?
	this->chunks.insert(map::value_type(key, chunk));
}

Chunk* World::get_chunk(Position::ChunkInWorld cp) const
{
	uint64_t key = chunk_key(cp.x, cp.y, cp.z);
	// TODO: what if this chunk has been changed by set_chunk?
	if(this->last_chunk != nullptr && key == this->last_key)
	{
		return this->last_chunk;
	}
	map::const_iterator it = this->chunks.find(key);
	Chunk* chunk;
	if(it == this->chunks.end())
	{
		return nullptr;
	}
	chunk = it->second;
	this->last_key = key;
	this->last_chunk = chunk;
	return chunk;
}

Chunk* World::get_or_make_chunk(Position::ChunkInWorld cp)
{
	Chunk* chunk = this->get_chunk(cp);
	if(chunk == nullptr)
	{
		chunk = new Chunk(cp);
		this->set_chunk(cp.x, cp.y, cp.z, chunk);
		this->gen_chunk(cp); // should this really be here?
	}
	// should this set last_key/last_chunk?
	return chunk;
}

void World::gen_chunk(const Position::ChunkInWorld& cp)
{
	//std::cout << "INFO: generating chunk: " << cp << "\n";
	Position::BlockInChunk min(0, 0, 0);
	Position::BlockInChunk max(CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1);
	this->gen_at(Position::BlockInWorld(cp, min), Position::BlockInWorld(cp, max));
}

void World::gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max)
{
	Position::BlockInWorld block_pos(BlockInWorld_type(0), 0, 0);
	for(BlockInWorld_type x = min.x; x <= max.x; ++x)
	{
		for(BlockInWorld_type z = min.z; z <= max.z; ++z)
		{
			for(BlockInWorld_type y = min.y; y <= max.y; ++y)
			{
				block_pos.x = x;
				block_pos.y = y;
				block_pos.z = z;
				if(y == -128 || (y > -64 && y < -32))
				{
					this->set_block(block_pos, new Block(1));
				}
				else if(x == -32 && y < 32)
				{
					this->set_block(block_pos, new Block(1));
				}
				else if(y == -32)
				{
					srand(static_cast<unsigned int>(x * z + x + z));
					int r = rand() % 9999;
					if(r == 0)
					{
						for(BlockInWorld_type y2 = -32; y2 < -24; ++y2)
						{
							block_pos.y = y2;
							this->set_block(block_pos, new Block(1));
						}
					}
				}
			}
		}
	}
}

void World::render_chunks()
{
	glUseProgram(Gfx::sp_cube);
	glUniformMatrix4fv(Gfx::vs_cube_matriks, 1, GL_FALSE, Gfx::matriks_ptr);

	const int render_distance = 2;

	Position::ChunkInWorld cp(Position::BlockInWorld(Game::instance->cam.position));
	Position::ChunkInWorld min(cp.x - render_distance, cp.y - render_distance, cp.z - render_distance);
	Position::ChunkInWorld max(cp.x + render_distance, cp.y + render_distance, cp.z + render_distance);
	for(int x = min.x; x <= max.x; ++x)
	{
		for(int y = min.y; y <= max.y; ++y)
		{
			for(int z = min.z; z <= max.z; ++z)
			{
				Chunk* chunk = this->get_or_make_chunk(Position::ChunkInWorld(x, y, z));
				chunk->render();
			}
		}
	}
}