#include "World.hpp"

#include <memory>
#include <random>
#include <utility>

#include "Block.hpp"
#include "BlockType.hpp"
#include "Player.hpp"
#include "chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

uint64_t key_hasher(const Position::ChunkInWorld& chunk_pos)
{
	uint32_t x_ = chunk_pos.x & 0x1FFFFF;
	uint32_t y_ = chunk_pos.y & 0x1FFFFF;
	uint32_t z_ = chunk_pos.z & 0x1FFFFF;
	uint64_t key =	  (static_cast<uint64_t>(x_) << 42)
					| (static_cast<uint64_t>(y_) << 21)
					| (static_cast<uint64_t>(z_))
				;
	return key;
}

World::World(const std::string& file_path)
	:
	chunks(0, key_hasher),
	last_chunk(nullptr),
	random_engine(0xFECA1),
	file(file_path, *this)
{
	file.load();
}

void World::set_block(const Position::BlockInWorld& block_pos, const Block& block)
{
	Position::ChunkInWorld chunk_pos(block_pos);
	std::shared_ptr<Chunk> chunk = get_or_make_chunk(chunk_pos);

	Position::BlockInChunk pos(block_pos);
	chunk->set_block(pos, block);
}

const Block& World::get_block_const(const Position::BlockInWorld& block_pos) const
{
	Position::ChunkInWorld chunk_pos(block_pos);
	std::shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		static const Block none = Block(BlockType::none);
		return none;
	}

	Position::BlockInChunk pos(block_pos);
	return chunk->get_block_const(pos);
}

Block& World::get_block_mutable(const Position::BlockInWorld& block_pos)
{
	Position::ChunkInWorld chunk_pos(block_pos);
	std::shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		static Block none = Block(BlockType::none); // TODO: this should be immutable
		return none;
	}

	Position::BlockInChunk pos(block_pos);
	return chunk->get_block_mutable(pos);
}

void World::set_chunk(const Position::ChunkInWorld& chunk_pos, std::shared_ptr<Chunk> chunk)
{
	if(last_chunk != nullptr && chunk_pos == last_key)
	{
		last_chunk = chunk;
	}
	chunks.insert({ chunk_pos, chunk });
}

std::shared_ptr<Chunk> World::get_chunk(const Position::ChunkInWorld& chunk_pos) const
{
	if(last_chunk != nullptr && chunk_pos == last_key)
	{
		return last_chunk;
	}
	auto i = chunks.find(chunk_pos);
	if(i == chunks.end())
	{
		return nullptr;
	}
	std::shared_ptr<Chunk> chunk = i->second;
	last_key = chunk_pos;
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
	// this does not set last_chunk/last_key because:
	// if the chunk is not null, get_chunk does it
	// if the chunk is null, set_chunk does it
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

	Position::BlockInWorld block_pos(static_cast<BlockInWorld_type>(0), 0, 0);
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
					set_block(block_pos, Block(BlockType::dots));
				}
				else if(y == -32)
				{
					auto r = distribution(random_engine);
					if(r == 0)
					{
						for(BlockInWorld_type y2 = y; y2 < y+8; ++y2)
						{
							block_pos.y = y2;
							set_block(block_pos, Block(BlockType::eye));
						}
					}
				}
			}
		}
	}
}

void World::step(const double delta_time)
{
	for(auto& p : players)
	{
		p.second->step(delta_time);
	}
}

std::shared_ptr<Player> World::add_player(const std::string& name)
{
	std::shared_ptr<Player> player = std::make_shared<Player>(name);
	players.emplace(name, player);
	return player;
}

std::shared_ptr<Player> World::get_player(const std::string& name)
{
	auto i = players.find(name);
	if(i == players.end())
	{
		return nullptr;
	}
	return i->second;
}

void World::save()
{
	file.save();
}
