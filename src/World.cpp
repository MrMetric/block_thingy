#include "World.hpp"

#include <memory>
#include <random>

#include <glad/glad.h>

#include "Block.hpp"
#include "Coords.hpp"
#include "chunk/Chunk.hpp"

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

uint64_t World::chunk_key(const Position::ChunkInWorld& chunk_pos)
{
	static_assert(sizeof(ChunkInWorld_type) <= 4, "update chunk_key for new ChunkInWorld_type size");
	uint32_t x_ = chunk_pos.x & 0xFFFFF;
	uint32_t y_ = chunk_pos.y & 0xFFFFF;
	uint32_t z_ = chunk_pos.z & 0xFFFFF;
	uint64_t key =	  (uint64_t(x_) << 40)
					| (uint64_t(y_) << 20)
					| (uint64_t(z_))
				;
	return key;
}

void World::set_block(const Position::BlockInWorld& block_pos, Block block)
{
	Position::ChunkInWorld chunk_pos(block_pos);
	std::shared_ptr<Chunk> chunk = get_or_make_chunk(chunk_pos);

	Position::BlockInChunk pos(block_pos);
	chunk->set_block(pos, block);
}

Block World::get_block(const Position::BlockInWorld& block_pos) const
{
	Position::ChunkInWorld chunk_pos(block_pos);
	std::shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		return Block(BlockType::none);
	}

	Position::BlockInChunk pos(block_pos);
	return chunk->get_block(pos);
}

void World::set_chunk(const Position::ChunkInWorld& chunk_pos, std::shared_ptr<Chunk> chunk)
{
	uint64_t key = chunk_key(chunk_pos);
	if(key == last_key)
	{
		last_chunk = chunk;
	}
	chunks.insert(map::value_type(key, chunk));
}

std::shared_ptr<Chunk> World::get_chunk(const Position::ChunkInWorld& chunk_pos) const
{
	uint64_t key = chunk_key(chunk_pos);
	if(last_chunk != nullptr && key == last_key)
	{
		return last_chunk;
	}
	auto it = chunks.find(key);
	if(it == chunks.end())
	{
		return nullptr;
	}
	std::shared_ptr<Chunk> chunk = it->second;
	last_key = key;
	last_chunk = chunk;
	return chunk;
}

std::shared_ptr<Chunk> World::get_or_make_chunk(const Position::ChunkInWorld& chunk_pos)
{
	std::shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		chunk = std::make_shared<Chunk>(chunk_pos, this);
		set_chunk(chunk_pos, chunk);
		gen_chunk(chunk_pos); // should this really be here?
	}
	// should this set last_key/last_chunk?
	return chunk;
}

void World::gen_chunk(const Position::ChunkInWorld& chunk_pos)
{
	Position::BlockInChunk min(0, 0, 0);
	Position::BlockInChunk max(CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1);
	gen_at(Position::BlockInWorld(chunk_pos, min), Position::BlockInWorld(chunk_pos, max));
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
					set_block(block_pos, Block(BlockType::test));
				}
				else if(x == -32 && y < 32)
				{
					set_block(block_pos, Block(BlockType::test));
				}
				else if(y == -32)
				{
					auto r = distribution(random_engine);
					if(r == 0)
					{
						for(BlockInWorld_type y2 = -32; y2 < -32+8; ++y2)
						{
							block_pos.y = y2;
							set_block(block_pos, Block(BlockType::test));
						}
					}
				}
			}
		}
	}
}
