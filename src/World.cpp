#include "World.hpp"

#include <functional>
#include <memory>
#include <random>

#include <GL/glew.h>

#include "Block.hpp"
#include "Chunk.hpp"
#include "Coords.hpp"
#include "Player.hpp"

World::World(GLint vs_cube_pos_mod)
	:
	vs_cube_pos_mod(vs_cube_pos_mod),
	last_key(~uint64_t(0)),
	last_chunk(nullptr),
	random_engine(0xFECA1)
{
}

World::~World()
{
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

void World::set_block(Position::BlockInWorld bwp, Block block)
{
	Position::ChunkInWorld cp(bwp);
	std::shared_ptr<Chunk> chunk = this->get_or_make_chunk(cp);

	Position::BlockInChunk bcp(bwp);
	chunk->set(bcp.x, bcp.y, bcp.z, block);
}

Block World::get_block(Position::BlockInWorld bwp) const
{
	Position::ChunkInWorld cp(bwp);
	std::shared_ptr<Chunk> chunk = this->get_chunk(cp);
	if(chunk == nullptr)
	{
		return Block();
	}

	Position::BlockInChunk bcp(bwp);
	return chunk->get_block(bcp);
}

void World::set_chunk(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z, std::shared_ptr<Chunk> chunk)
{
	uint64_t key = chunk_key(x, y, z);
	if(key == this->last_key)
	{
		this->last_chunk = chunk;
	}
	this->chunks.insert(map::value_type(key, chunk));
}

std::shared_ptr<Chunk> World::get_chunk(Position::ChunkInWorld cp) const
{
	uint64_t key = chunk_key(cp.x, cp.y, cp.z);
	if(this->last_chunk != nullptr && key == this->last_key)
	{
		return this->last_chunk;
	}
	auto it = this->chunks.find(key);
	if(it == this->chunks.end())
	{
		return nullptr;
	}
	std::shared_ptr<Chunk> chunk = it->second;
	this->last_key = key;
	this->last_chunk = chunk;
	return chunk;
}

std::shared_ptr<Chunk> World::get_or_make_chunk(Position::ChunkInWorld cp)
{
	std::shared_ptr<Chunk> chunk = this->get_chunk(cp);
	if(chunk == nullptr)
	{
		chunk = std::make_shared<Chunk>(cp, this);
		this->set_chunk(cp.x, cp.y, cp.z, chunk);
		this->gen_chunk(cp); // should this really be here?
	}
	// should this set last_key/last_chunk?
	return chunk;
}

void World::gen_chunk(const Position::ChunkInWorld& cp)
{
	Position::BlockInChunk min(0, 0, 0);
	Position::BlockInChunk max(CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1);
	this->gen_at(Position::BlockInWorld(cp, min), Position::BlockInWorld(cp, max));
}

void World::gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max)
{
	static std::uniform_int_distribution<uint_fast16_t> distribution(0, 999);

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
					this->set_block(block_pos, Block(1));
				}
				else if(x == -32 && y < 32)
				{
					this->set_block(block_pos, Block(1));
				}
				else if(y == -32)
				{
					auto r = distribution(this->random_engine);
					if(r == 0)
					{
						for(BlockInWorld_type y2 = -32; y2 < -24; ++y2)
						{
							block_pos.y = y2;
							this->set_block(block_pos, Block(1));
						}
					}
				}
			}
		}
	}
}
