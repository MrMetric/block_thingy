#include "World.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <queue>
#include <stdint.h>
#include <tuple>
#include <unordered_set>
#include <utility>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>

#include "Game.hpp"
#include "Player.hpp"
#include "block/Base.hpp"
#include "block/BlockRegistry.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "chunk/mesh/GreedyMesher.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::shared_ptr;
using std::unique_ptr;

using Position::BlockInChunk;
using Position::BlockInWorld;
using Position::ChunkInWorld;

template <typename T>
uint64_t position_hasher(const T& pos)
{
	// x has 1 more bit than y/z because there is room for it
	// y/z are both 21 bits
	// 64 - 21*2 = 22
	const uint32_t x = pos.x & 0x3FFFFF;
	const uint32_t y = pos.y & 0x1FFFFF;
	const uint32_t z = pos.z & 0x1FFFFF;
	return	  (static_cast<uint64_t>(x) << 42)
			| (static_cast<uint64_t>(y) << 21)
			| (static_cast<uint64_t>(z))
		;
}

World::World(const string& file_path)
	:
	mesher(std::make_unique<GreedyMesher>()),
	ticks(0),
	chunks(0, position_hasher<ChunkInWorld>),
	last_chunk(nullptr),
	chunks_to_save(0, position_hasher<ChunkInWorld>),
	file(file_path, *this)
{
}

void World::set_block(const BlockInWorld& block_pos, unique_ptr<Block::Base> block_ptr)
{
	const ChunkInWorld chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_or_make_chunk(chunk_pos);

	const Block::Base& old_block = get_block(block_pos);
	const bool old_is_opaque = old_block.is_opaque();
	const Graphics::Color old_color = old_block.color();
	// old_block is invalid after the call to set_block

	const BlockInChunk pos(block_pos);
	chunk->set_block(pos, std::move(block_ptr));
	const Block::Base& block = chunk->get_block(pos);

	chunks_to_save.emplace(chunk_pos);

	if(block.is_opaque() && !old_is_opaque)
	{
		sub_light(block_pos);
	}

	const Graphics::Color color = block.color();
	if(old_color != color)
	{
		if(old_color != 0)
		{
			sub_light(block_pos);
		}
		add_light(block_pos, color);
	}

	if(block.is_opaque() != old_is_opaque)
	{
		update_light_around(block_pos);
	}
}

const Block::Base& World::get_block(const BlockInWorld& block_pos) const
{
	const ChunkInWorld chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		static const std::unique_ptr<Block::Base> none = Game::instance->block_registry.make(BlockType::none);
		return *none;
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
	if(color == 0)
	{
		return;
	}

	// see https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
	light_add.emplace(block_pos);
	process_light_add();
}

void World::process_light_add()
{
	while(!light_add.empty())
	{
		const BlockInWorld pos = light_add.front();
		light_add.pop();
		const Graphics::Color color = get_light(pos) - 1;
		if(color == 0)
		{
			continue;
		}

		auto fill = [this, &pos, &color](const int8_t x, const int8_t y, const int8_t z)
		{
			const BlockInWorld pos2{pos.x + x, pos.y + y, pos.z + z};
			if(get_block(pos2).is_opaque())
			{
				return;
			}
			Graphics::Color color2 = get_light(pos2);
			bool set = false;
			for(uint_fast8_t i = 0; i < 3; ++i)
			{
				if(color2[i] < color[i])
				{
					color2[i] = color[i];
					set = true;
				}
			}
			if(set)
			{
				set_light(pos2, color2);
				light_add.emplace(pos2);
			}
		};

		fill( 0,  0, -1);
		fill( 0,  0, +1);
		fill( 0, -1,  0);
		fill( 0, +1,  0);
		fill(-1,  0,  0);
		fill(+1,  0,  0);
	}
}

void World::sub_light(const BlockInWorld& block_pos)
{
	Graphics::Color color = get_light(block_pos);
	set_light(block_pos, {0, 0, 0});

	std::queue<std::tuple<BlockInWorld, Graphics::Color>> q;
	q.emplace(block_pos, color);
	while(!q.empty())
	{
		const BlockInWorld pos = std::get<0>(q.front());
		const Graphics::Color color = std::get<1>(q.front());
		q.pop();

		auto fill = [this, &q, &pos](const Graphics::Color& color, const int8_t x, const int8_t y, const int8_t z)
		{
			const BlockInWorld pos2{pos.x + x, pos.y + y, pos.z + z};
			Graphics::Color color2 = get_light(pos2);
			Graphics::Color color_set = color2;
			Graphics::Color color_put(0, 0, 0);

			bool set = false;
			for(uint_fast8_t i = 0; i < 3; ++i)
			{
				if(color2[i] != 0 && color2[i] < color[i])
				{
					color_set[i] = 0;
					color_put[i] = color2[i];
					set = true;
				}
				else if(color2[i] >= color[i])
				{
					light_add.emplace(pos2);
				}
			}
			if(set)
			{
				set_light(pos2, color_set);
				q.emplace(pos2, color_put);
			}
		};

		fill(color,  0,  0, -1);
		fill(color,  0,  0, +1);
		fill(color,  0, -1,  0);
		fill(color,  0, +1,  0);
		fill(color, -1,  0,  0);
		fill(color, +1,  0,  0);
	}
}

void World::update_light_around(const BlockInWorld& block_pos)
{
	#define a(x_, y_, z_) light_add.emplace(block_pos.x + x_, block_pos.y + y_, block_pos.z + z_)
	a( 0,  0, -1);
	a( 0,  0, +1);
	a( 0, -1,  0);
	a( 0, +1,  0);
	a(-1,  0,  0);
	a(+1,  0,  0);
	#undef a

	process_light_add();
}

void World::set_chunk(const ChunkInWorld& chunk_pos, shared_ptr<Chunk> chunk)
{
	if(last_chunk != nullptr && chunk_pos == last_key)
	{
		last_chunk = chunk;
	}

	if(!chunks.emplace(chunk_pos, chunk).second)
	{
		chunks[chunk_pos] = chunk;
	}
	if(chunk == nullptr)
	{
		return;
	}
	chunk->update_neighbors();

	// update light at chunk sides to make it flow into the new chunk
	{
		BlockInWorld pos1(chunk_pos, {0, 0, 0});
		BlockInWorld pos;
		for(uint_fast8_t i1 = 0; i1 < 3; ++i1)
		{
			uint_fast8_t i2 = (i1 + 1) % 3;
			uint_fast8_t i3 = (i1 + 2) % 3;
			for(pos[i1] = pos1[i1]; pos[i1] < pos1[i1] + CHUNK_SIZE; ++pos[i1])
			for(pos[i2] = pos1[i2]; pos[i2] < pos1[i2] + CHUNK_SIZE; ++pos[i2])
			{
				BlockInWorld pos2(pos);

				pos2[i3] = pos1[i3] - 1;
				light_add.emplace(pos2);

				pos2[i3] = pos1[i3] + CHUNK_SIZE;
				light_add.emplace(pos2);
			}
		}
		process_light_add();
	}

	// update light in chunk
	{
		BlockInChunk pos;
		for(pos.x = 0; pos.x < CHUNK_SIZE; ++pos.x)
		for(pos.y = 0; pos.y < CHUNK_SIZE; ++pos.y)
		for(pos.z = 0; pos.z < CHUNK_SIZE; ++pos.z)
		{
			const Block::Base& block = chunk->get_block(pos);
			const Graphics::Color color = block.color();
			if(color != 0)
			{
				add_light({chunk_pos, pos}, color);
			}
		}
	}
}

shared_ptr<Chunk> World::get_chunk(const ChunkInWorld& chunk_pos) const
{
	if(last_chunk != nullptr && chunk_pos == last_key)
	{
		return last_chunk;
	}

	const auto i = chunks.find(chunk_pos);
	if(i == chunks.cend())
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

static double sum_octaves(
	const glm::dvec2& P,
	const double base_freq,
	const double freq_mul,
	const uint_fast8_t octave_count
)
{
	double val = 0;
	double freq = base_freq;
	for(uint_fast8_t i = 0; i < octave_count; i++)
	{
		const glm::dvec2 Pm(P.x * freq, P.y * freq);
		val += std::abs(glm::simplex(Pm) / freq);
		freq *= freq_mul;
	}
	return val;
};

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

			// https://www.shadertoy.com/view/Xl3GWS
			auto get_max_y = [](const BlockInWorld::value_type x, const BlockInWorld::value_type z) -> double
			{
				// coords must not be (0, 0) (it makes this function always return 0)
				const glm::dvec2 coords = (x == 0 && z == 0) ? glm::dvec2(0.0001) : glm::dvec2(x, z) / 1024.0;
				const glm::dvec2 n(-sum_octaves(coords, 1, 2.07, 8));

				const double a = n.x * n.y;
				const double b = glm::mod(a, 1.0);
				const auto d = static_cast<uint_fast8_t>(glm::mod(ceil(a), 2.0));
				return (d == 0 ? b : 1 - b) - 1;
			};

			auto max_y = max.y <= -m ? max.y : std::min(max.y, static_cast<BlockInWorld::value_type>(get_max_y(x, z) * m));

			block_pos.x = x;
			block_pos.z = z;
			for(auto y = min.y; y <= max_y; ++y)
			{
				block_pos.y = y;

				const ChunkInWorld chunkpos(block_pos);
				const BlockType t = y > -m / 2 ? BlockType::white : BlockType::black;
				set_block(block_pos, Game::instance->block_registry.make(t));
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
	ticks += 1;
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
	const auto i = players.find(name);
	if(i == players.cend())
	{
		return nullptr;
	}
	return i->second;
}

void World::save()
{
	file.save_world();
	file.save_players();

	while(!chunks_to_save.empty())
	{
		const auto i = chunks_to_save.cbegin();
		const ChunkInWorld position = *i;
		chunks_to_save.erase(i);
		shared_ptr<Chunk> chunk = get_chunk(position);
		if(chunk != nullptr)
		{
			file.save_chunk(*chunk);
		}
	}
}

uint_fast64_t World::get_ticks()
{
	return ticks;
}

double World::get_time()
{
	return ticks / 60.0;
}
