#include "World.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <queue>
#include <stdint.h>
#include <utility>

#include <glm/gtc/noise.hpp>

#include "Player.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "chunk/mesh/GreedyMesher.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

using std::shared_ptr;
using std::string;

using Position::BlockInChunk;
using Position::BlockInWorld;
using Position::ChunkInWorld;

template <typename T>
uint64_t position_hasher(const T& pos)
{
	uint32_t x = pos.x & 0x1FFFFF;
	uint32_t y = pos.y & 0x1FFFFF;
	uint32_t z = pos.z & 0x1FFFFF;
	uint64_t key =	  (static_cast<uint64_t>(x) << 42)
					| (static_cast<uint64_t>(y) << 21)
					| (static_cast<uint64_t>(z))
				;
	return key;
}

World::World(const string& file_path)
	:
	mesher(std::make_unique<GreedyMesher>()),
	chunks(0, position_hasher<ChunkInWorld>),
	last_chunk(nullptr),
	file(file_path, *this)
{
}

void World::set_block(const BlockInWorld& block_pos, const Block::Block& block)
{
	ChunkInWorld chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_or_make_chunk(chunk_pos);

	BlockInChunk pos(block_pos);
	chunk->set_block(pos, block);

	if(block.type() == BlockType::light_test_red)
	{
		add_light(block_pos, Graphics::Color(16, 0, 0));
	}
	else if(block.type() == BlockType::light_test_green)
	{
		add_light(block_pos, Graphics::Color(0, 16, 0));
	}
	else if(block.type() == BlockType::light_test_blue)
	{
		add_light(block_pos, Graphics::Color(0, 0, 16));
	}
	else if(block.type() == BlockType::light_test_yellow)
	{
		add_light(block_pos, Graphics::Color(16, 16, 0));
	}
	else if(block.type() == BlockType::light_test_cyan)
	{
		add_light(block_pos, Graphics::Color(0, 16, 16));
	}
	else if(block.type() == BlockType::light_test_pink)
	{
		add_light(block_pos, Graphics::Color(16, 0, 16));
	}
	else if(block.type() == BlockType::light_test_white)
	{
		add_light(block_pos, Graphics::Color(16, 16, 16));
	}

	chunks_to_save.insert(chunk_pos);
}

Block::Block World::get_block(const BlockInWorld& block_pos) const
{
	ChunkInWorld chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		static const Block::Block none = Block::Block(BlockType::none);
		return none;
	}

	BlockInChunk pos(block_pos);
	return chunk->get_block(pos);
}

Graphics::Color World::get_light(const BlockInWorld& block_pos) const
{
	const ChunkInWorld chunk_pos(block_pos);
	const shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		return Graphics::Color();
	}
	return chunk->get_light(BlockInChunk(block_pos));
}

void World::set_light(const BlockInWorld& block_pos, const Graphics::Color& color)
{
	const ChunkInWorld chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		// TODO?
		return;
	}
	chunk->set_light(BlockInChunk(block_pos), color);
}

void World::add_light(const BlockInWorld& block_pos, const Graphics::Color& color)
{
	set_light(block_pos, color);

	// see https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
	std::queue<std::tuple<BlockInWorld, glm::dvec3>> q;
	q.emplace(block_pos, glm::dvec3(0, 0, 0));
	std::unordered_map<BlockInWorld, bool, std::function<uint64_t(BlockInWorld)>> visited(0, position_hasher<BlockInWorld>);
	while(!q.empty())
	{
		const auto pos = std::get<0>(q.front());
		const auto traveled = std::get<1>(q.front());
		q.pop();

		auto fill = [this, &block_pos, &q, &visited, &pos, &traveled](Graphics::Color color, int8_t x, int8_t y, int8_t z)
		{
			const BlockInWorld pos2{pos.x + x, pos.y + y, pos.z + z};
			// emplace failed = key already exists = position has been visited
			if(!visited.emplace(pos2, true).second)
			{
				return;
			}
			if(get_block(pos2).is_opaque())
			{
				return;
			}
			const glm::dvec3 traveled2(traveled.x + std::abs(x), traveled.y + std::abs(y), traveled.z + std::abs(z));
			const double distance = glm::length(traveled2);
			color -= std::round(distance);
			if(color < 1)
			{
				return;
			}
			const auto color2 = get_light(pos2) + color;
			set_light(pos2, color2);
			q.emplace(pos2, traveled2);
		};

		fill(color,  0,  0, -1);
		fill(color,  0,  0, +1);
		fill(color,  0, +1,  0);
		fill(color,  0, -1,  0);
		fill(color, -1,  0,  0);
		fill(color, +1,  0,  0);
	}
}

void World::set_chunk(const ChunkInWorld& chunk_pos, shared_ptr<Chunk> chunk)
{
	if(last_chunk != nullptr && chunk_pos == last_key)
	{
		last_chunk = chunk;
	}
	chunks.insert({ chunk_pos, chunk });
	chunk->update_neighbors();
}

shared_ptr<Chunk> World::get_chunk(const ChunkInWorld& chunk_pos) const
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
	shared_ptr<Chunk> chunk = i->second;
	last_key = chunk_pos;
	last_chunk = chunk;
	return chunk;
}

// this does not set last_chunk/last_key because:
// if the chunk is not null, get_chunk does it
// if the chunk is null, set_chunk does it
shared_ptr<Chunk> World::get_or_make_chunk(const ChunkInWorld& chunk_pos)
{
	shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk != nullptr)
	{
		return chunk;
	}

	chunk = file.load_chunk(chunk_pos);
	if(chunk != nullptr)
	{
		set_chunk(chunk_pos, chunk);
		return chunk;
	}

	{
		chunk = std::make_shared<Chunk>(chunk_pos, *this);
		set_chunk(chunk_pos, chunk);
		gen_chunk(chunk_pos); // should this really be here?
		return chunk;
	}
}

void World::gen_chunk(const ChunkInWorld& chunk_pos)
{
	BlockInChunk min(0, 0, 0);
	BlockInChunk max(CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1);
	gen_at(BlockInWorld(chunk_pos, min), BlockInWorld(chunk_pos, max));
}

void World::gen_at(const BlockInWorld& min, const BlockInWorld& max)
{
	BlockInWorld block_pos(0, 0, 0);
	for(auto x = min.x; x <= max.x; ++x)
	{
		for(auto z = min.z; z <= max.z; ++z)
		{
			const double m = 20;
			if(min.y > 0)
			{
				continue;
			}

			const uint_fast8_t roughness = 8;
			auto turbulence = [](const glm::dvec2& P)
			{
				double val = 0;
				double freq = 1;
				for(uint_fast8_t i = 0; i < roughness; i++)
				{
					const glm::dvec2 Pm(P.x * freq, P.y * freq);
					val += glm::abs(glm::simplex(Pm) / freq);
					freq *= 2.07;
				}
				return val;
			};

			auto get_max_y = [&turbulence, x, z, m]()
			{
				const glm::dvec2 n(-turbulence(glm::dvec2(x, z) / 1024.0));
				const double a = n.x * n.y;
				const double b = glm::mod(a, 1.0);
				const double d = glm::mod(ceil(a), 2.0);
				return static_cast<BlockInWorld::value_type>(((d == 0 ? b : d - b) - 1) * m);
			};

			auto max_y = max.y <= -m ? max.y : std::min(max.y, get_max_y());

			for(auto y = min.y; y <= max_y; ++y)
			{
				block_pos.x = x;
				block_pos.y = y;
				block_pos.z = z;

				const ChunkInWorld chunkpos(block_pos);
				const BlockType t = y > -m / 2 ? BlockType::white : BlockType::black;
				set_block(block_pos, Block::Block(t));
			}
		}
	}
}

void World::step(double delta_time)
{
	delta_time = 1.0 / 60.0;
	for(auto& p : players)
	{
		p.second->step(delta_time);
	}
}

shared_ptr<Player> World::add_player(const string& name)
{
	shared_ptr<Player> player = file.load_player(name);
	if(player == nullptr)
	{
		player = std::make_shared<Player>(name);
	}
	players.emplace(name, player);
	return player;
}

shared_ptr<Player> World::get_player(const string& name)
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
	file.save_players();
	while(!chunks_to_save.empty())
	{
		const ChunkInWorld position = *chunks_to_save.begin();
		chunks_to_save.erase(chunks_to_save.begin());
		shared_ptr<Chunk> chunk = get_chunk(position);
		if(chunk != nullptr)
		{
			file.save_chunk(*chunk);
		}
	}
}
