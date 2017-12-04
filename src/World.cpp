#include "World.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <stdint.h>
#include <tuple>
#include <unordered_set>
#include <utility>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>

#include "Player.hpp"
#include "block/Base.hpp"
#include "block/BlockRegistry.hpp"
#include "block/Enum/Type.hpp"
#include "chunk/Chunk.hpp"
#include "chunk/Mesher/Base.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"
#include "position/hash.hpp"
#include "shim/make_unique.hpp"
#include "storage/WorldFile.hpp"
#include "util/ThreadThingy.hpp"

using std::string;
using std::shared_ptr;
using std::unique_ptr;

using Position::BlockInChunk;
using Position::BlockInWorld;
using Position::ChunkInWorld;

struct World::impl
{
	impl
	(
		World& world,
		const fs::path& file_path
	)
	:
		world(world),
		file(file_path, world),
		gen_thread([this, &world](const ChunkInWorld& pos)
		{
			shared_ptr<Chunk> chunk = std::make_shared<Chunk>(pos, world);
			gen_chunk(chunk);
			generated_chunks.enqueue(chunk);
		}, 2, Position::hasher<ChunkInWorld>),
		load_thread([this](const ChunkInWorld& pos)
		{
			shared_ptr<Chunk> chunk(file.load_chunk(pos));
			assert(chunk != nullptr);
			loaded_chunks.enqueue(chunk);
			mesh_thread.enqueue(chunk);
		}, 2, Position::hasher<ChunkInWorld>),
		mesh_thread([this](shared_ptr<Chunk>& chunk)
		{
			assert(chunk != nullptr);
			chunk->update();
			mesh_thread.dequeue(chunk);
		}, 2)
	{
	}

	World& world;

	Position::unordered_map_t<ChunkInWorld, shared_ptr<Chunk>> chunks;
	mutable std::mutex chunks_mutex;

	std::unordered_set<ChunkInWorld, Position::hasher_struct<ChunkInWorld>> chunks_to_save;

	std::unordered_map<string, shared_ptr<Player>> players;

	Storage::WorldFile file;

	void update_chunk_neighbors
	(
		const ChunkInWorld&,
		bool thread = true
	);
	void update_chunk_neighbors
	(
		const ChunkInWorld&,
		const BlockInChunk&,
		bool thread = true
	);
	void update_chunk_neighbor
	(
		const ChunkInWorld&,
		const ChunkInWorld&,
		bool thread = true
	);

	Util::ThreadThingy<ChunkInWorld, Position::hasher_t<ChunkInWorld>> gen_thread;
	moodycamel::ConcurrentQueue<shared_ptr<Chunk>> generated_chunks;
	void gen_chunk(shared_ptr<Chunk>) const;

	Util::ThreadThingy<ChunkInWorld, Position::hasher_t<ChunkInWorld>> load_thread;
	moodycamel::ConcurrentQueue<shared_ptr<Chunk>> loaded_chunks;

	Util::ThreadThingy<shared_ptr<Chunk>> mesh_thread;

	std::queue<BlockInWorld> blocklight_add;
	void add_blocklight(const BlockInWorld&, const Graphics::Color&, bool save);
	void process_blocklight_add();
	void sub_blocklight(const BlockInWorld&);
	void update_blocklight_around(const BlockInWorld&);
};

World::World
(
	const fs::path& file_path,
	Block::BlockRegistry& block_registry,
	unique_ptr<Mesher::Base> mesher
)
:
	block_registry(block_registry),
	mesher(std::move(mesher)),
	ticks(0),
	pImpl(std::make_unique<impl>
	(
		*this,
		file_path
	))
{
}

World::~World()
{
	// this does not work in ~impl
	pImpl->gen_thread.stop();
	pImpl->load_thread.stop();
	pImpl->mesh_thread.stop();
}

static bool does_affect_light(const Block::Base& block)
{
	if(block.is_opaque())
	{
		return true;
	}
	static Graphics::Color max(Graphics::Color::max);
	if(block.is_translucent() && block.light_filter() != max)
	{
		return true;
	}
	return false;
}

void World::set_block
(
	const BlockInWorld& block_pos,
	shared_ptr<Block::Base> block,
	bool thread
)
{
	if(block == nullptr)
	{
		throw std::invalid_argument("block must not be null");
	}

	const ChunkInWorld chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_or_make_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		// TODO: handle this better
		return;
	}

	const shared_ptr<Block::Base> old_block = get_block(block_pos);
	if(old_block == block)
	{
		return;
	}

	const BlockInChunk pos(block_pos);
	chunk->set_block(pos, block);
	pImpl->chunks_to_save.emplace(chunk_pos);

	const bool old_affects_light = does_affect_light(*old_block);
	const bool affects_light = does_affect_light(*block);

	const Graphics::Color old_light = old_block->light();
	const Graphics::Color light = block->light();

	// TODO: these checks might not work (a filter block could be overwritten by a different filter block)
	if(affects_light && !old_affects_light)
	{
		pImpl->sub_blocklight(block_pos);
	}

	if(old_light != light)
	{
		if(old_light != 0)
		{
			pImpl->sub_blocklight(block_pos);
		}
		pImpl->add_blocklight(block_pos, light, false);
	}

	if(affects_light != old_affects_light)
	{
		pImpl->update_blocklight_around(block_pos);
	}

	pImpl->update_chunk_neighbors(chunk_pos, pos, thread);
	if(thread)
	{
		pImpl->mesh_thread.enqueue(chunk);
	}
	else
	{
		chunk->update();
	}
}

const shared_ptr<Block::Base> World::get_block(const BlockInWorld& block_pos) const
{
	const ChunkInWorld chunk_pos(block_pos);
	const shared_ptr<const Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		static const shared_ptr<Block::Base> none = block_registry.get_default(Block::Enum::Type::none);
		return none;
	}

	BlockInChunk pos(block_pos);
	return chunk->get_block(pos);
}

shared_ptr<Block::Base> World::get_block(const BlockInWorld& block_pos)
{
	const ChunkInWorld chunk_pos(block_pos);
	const shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		static /*const*/ shared_ptr<Block::Base> none = block_registry.get_default(Block::Enum::Type::none);
		return none;
	}

	BlockInChunk pos(block_pos);
	return chunk->get_block(pos);
}

Graphics::Color World::get_blocklight(const BlockInWorld& block_pos) const
{
	const ChunkInWorld chunk_pos(block_pos);
	const shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		return {0, 0, 0};
	}
	return chunk->get_blocklight(BlockInChunk(block_pos));
}

void World::set_blocklight
(
	const BlockInWorld& block_pos,
	const Graphics::Color& color,
	bool save
)
{
	const ChunkInWorld chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		// TODO?: handle this better
		return;
	}
	const BlockInChunk pos(block_pos);
	chunk->set_blocklight(pos, color);

	// update light in neighboring chunks
	{
		glm::tvec3<bool> xyz(false, false, false);
		glm::tvec3<bool> zero(glm::uninitialize);
		auto do_it = [this, &chunk_pos, &color, &pos, &zero](const glm::tvec3<bool>& xyz)
		{
			ChunkInWorld chunk_pos_2 = chunk_pos;
			if(xyz.x) chunk_pos_2.x += (zero.x ? -1 : 1);
			if(xyz.y) chunk_pos_2.y += (zero.y ? -1 : 1);
			if(xyz.z) chunk_pos_2.z += (zero.z ? -1 : 1);
			auto chunk2 = get_chunk(chunk_pos_2);
			if(chunk2 != nullptr)
			{
				glm::ivec3 pos2(glm::uninitialize);
				pos2.x = xyz.x ? (zero.x ? CHUNK_SIZE : -1) : pos.x;
				pos2.y = xyz.y ? (zero.y ? CHUNK_SIZE : -1) : pos.y;
				pos2.z = xyz.z ? (zero.z ? CHUNK_SIZE : -1) : pos.z;
				chunk2->set_texbuflight(pos2, color);
			}
		};
		for(uint_fast8_t i = 0; i < 3; ++i)
		{
			if(pos[i] == 0 || pos[i] == CHUNK_SIZE - 1)
			{
				xyz[i] = true;
				zero[i] = (pos[i] == 0);
			}
		}
		for(uint_fast8_t i = 0; i < 3; ++i)
		{
			const uint_fast8_t j = (i + 1) % 3;
			if(xyz[i])
			{
				glm::tvec3<bool> xyz2(false, false, false);
				xyz2[i] = true;
				do_it(xyz2);
				if(xyz[j])
				{
					xyz2[j] = true;
					do_it(xyz2);
				}
			}
		}
		if(xyz.x && xyz.y && xyz.z)
		{
			do_it({true, true, true});
		}
	}

	if(save)
	{
		pImpl->chunks_to_save.emplace(chunk_pos);
	}
}

void World::update_blocklight
(
	const BlockInWorld& block_pos,
	const bool save
)
{
	update_blocklight(block_pos, get_block(block_pos)->light(), save);
}

void World::update_blocklight
(
	const BlockInWorld& block_pos,
	const Graphics::Color& color,
	const bool save
)
{
	pImpl->sub_blocklight(block_pos);
	pImpl->add_blocklight(block_pos, color, save);
}

void World::impl::add_blocklight
(
	const BlockInWorld& block_pos,
	const Graphics::Color& color,
	const bool save
)
{
	world.set_blocklight(block_pos, color, save);
	if(color == 0)
	{
		return;
	}

	// see https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
	blocklight_add.emplace(block_pos);
	process_blocklight_add();
}

void World::impl::process_blocklight_add()
{
	while(!blocklight_add.empty())
	{
		const BlockInWorld pos = blocklight_add.front();
		blocklight_add.pop();
		const Graphics::Color color = world.get_blocklight(pos) - 1;
		if(color == 0)
		{
			continue;
		}

		auto fill =
		[
			this,
			&pos
		]
		(
			const int8_t x,
			const int8_t y,
			const int8_t z,
			Graphics::Color color
		)
		{
			const BlockInWorld pos2{pos.x + x, pos.y + y, pos.z + z};
			const shared_ptr<Chunk> chunk = world.get_chunk(ChunkInWorld(pos2));
			if(chunk == nullptr)
			{
				return;
			}
			const BlockInChunk pos2b(pos2);
			const shared_ptr<Block::Base> block = chunk->get_block(pos2b);
			if(block->is_opaque())
			{
				return;
			}
			if(block->is_translucent())
			{
				const Graphics::Color f = block->light_filter();
				color.r = std::min(color.r, f.r);
				color.g = std::min(color.g, f.g);
				color.b = std::min(color.b, f.b);
			}
			Graphics::Color color2 = chunk->get_blocklight(pos2b);
			bool set = false;
			if(color2.r < color.r) { color2.r = color.r; set = true; }
			if(color2.g < color.g) { color2.g = color.g; set = true; }
			if(color2.b < color.b) { color2.b = color.b; set = true; }
			if(set)
			{
				world.set_blocklight(pos2, color2, false);
				blocklight_add.emplace(pos2);
			}
		};

		fill( 0,  0, -1, color);
		fill( 0,  0, +1, color);
		fill( 0, -1,  0, color);
		fill( 0, +1,  0, color);
		fill(-1,  0,  0, color);
		fill(+1,  0,  0, color);
	}
}

void World::impl::sub_blocklight(const BlockInWorld& block_pos)
{
	const Graphics::Color color = world.get_blocklight(block_pos);
	if(color == 0)
	{
		return;
	}
	world.set_blocklight(block_pos, {0, 0, 0}, false);

	std::queue<std::tuple<BlockInWorld, Graphics::Color>> q;
	q.emplace(block_pos, color);
	while(!q.empty())
	{
		const BlockInWorld pos = std::get<0>(q.front());
		const Graphics::Color color = std::get<1>(q.front());
		q.pop();

		auto fill =
		[
			this,
			&q,
			&pos
		]
		(
			const Graphics::Color& color,
			const int8_t x,
			const int8_t y,
			const int8_t z
		)
		{
			const BlockInWorld pos2{pos.x + x, pos.y + y, pos.z + z};
			Graphics::Color color2 = world.get_blocklight(pos2);
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
					blocklight_add.emplace(pos2);
				}
			}
			if(set)
			{
				world.set_blocklight(pos2, color_set, false);
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

void World::impl::update_blocklight_around(const BlockInWorld& block_pos)
{
	#define a(x_, y_, z_) blocklight_add.emplace(block_pos.x + x_, block_pos.y + y_, block_pos.z + z_)
	a( 0,  0, -1);
	a( 0,  0, +1);
	a( 0, -1,  0);
	a( 0, +1,  0);
	a(-1,  0,  0);
	a(+1,  0,  0);
	#undef a
	process_blocklight_add();
}

void World::set_chunk(const ChunkInWorld& chunk_pos, shared_ptr<Chunk> chunk)
{
	const shared_ptr<Chunk> prev_chunk = get_chunk(chunk_pos);
	if(prev_chunk == chunk)
	{
		return;
	}
	{
		std::lock_guard<std::mutex> g(pImpl->chunks_mutex);
		pImpl->chunks.insert_or_assign(chunk_pos, chunk);
	}
	if(chunk == nullptr)
	{
		return;
	}

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
				pImpl->blocklight_add.emplace(pos2);

				pos2[i3] = pos1[i3] + CHUNK_SIZE;
				pImpl->blocklight_add.emplace(pos2);
			}
		}
		pImpl->process_blocklight_add();
	}

	// update light in chunk
	{
		BlockInChunk pos;
		for(pos.x = 0; pos.x < CHUNK_SIZE; ++pos.x)
		for(pos.y = 0; pos.y < CHUNK_SIZE; ++pos.y)
		for(pos.z = 0; pos.z < CHUNK_SIZE; ++pos.z)
		{
			shared_ptr<Block::Base> block = chunk->get_block(pos);

			const Graphics::Color light = block->light();
			if(light != 0)
			{
				pImpl->add_blocklight({chunk_pos, pos}, light, false);
			}
		}
	}

	{
		glm::ivec3 pos2;
		for(pos2.x = -1; pos2.x < CHUNK_SIZE + 1; ++pos2.x)
		for(pos2.y = -1; pos2.y < CHUNK_SIZE + 1; ++pos2.y)
		for(pos2.z = -1; pos2.z < CHUNK_SIZE + 1; ++pos2.z)
		{
			ChunkInWorld chunk_pos_2 = chunk_pos;
			BlockInChunk pos;
			for(uint_fast8_t i = 0; i < 3; ++i)
			{
				if(pos2[i] == -1)
				{
					chunk_pos_2[i] -= 1;
					pos[i] = CHUNK_SIZE - 1;
				}
				else if(pos2[i] == CHUNK_SIZE)
				{
					chunk_pos_2[i] += 1;
					pos[i] = 0;
				}
				else
				{
					pos[i] = static_cast<BlockInChunk::value_type>(pos2[i]);
				}
			}
			if(chunk_pos_2 == chunk_pos)
			{
				continue;
			}
			const auto chunk2 = get_chunk(chunk_pos_2);
			if(chunk2 != nullptr)
			{
				chunk->set_texbuflight(pos2, chunk2->get_blocklight(pos));
			}
		}
	}

	pImpl->mesh_thread.enqueue(chunk);
	pImpl->update_chunk_neighbors(chunk_pos);
}

shared_ptr<Chunk> World::get_chunk(const ChunkInWorld& chunk_pos) const
{
	std::lock_guard<std::mutex> g(pImpl->chunks_mutex);
	const auto i = pImpl->chunks.find(chunk_pos);
	if(i == pImpl->chunks.cend())
	{
		return nullptr;
	}
	return i->second;
}

shared_ptr<Chunk> World::get_or_make_chunk(const ChunkInWorld& chunk_pos)
{
	shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk != nullptr)
	{
		return chunk;
	}

	if(pImpl->file.has_chunk(chunk_pos))
	{
		pImpl->load_thread.enqueue(chunk_pos);
	}
	else
	{
		pImpl->gen_thread.enqueue(chunk_pos);
	}

	return nullptr;
}

void World::step(double delta_time)
{
	delta_time = 1.0 / 60.0; // TODO

	shared_ptr<Chunk> chunk;
	while(pImpl->loaded_chunks.try_dequeue(chunk))
	{
		ChunkInWorld pos = chunk->get_position();
		set_chunk(pos, chunk);
		pImpl->load_thread.dequeue(pos);
	}
	while(pImpl->generated_chunks.try_dequeue(chunk))
	{
		ChunkInWorld pos = chunk->get_position();
		set_chunk(pos, chunk);
		pImpl->gen_thread.dequeue(pos);
		pImpl->chunks_to_save.emplace(pos);
	}

	for(auto& p : pImpl->players)
	{
		p.second->step(delta_time);
	}

	ticks += 1;
}

shared_ptr<Player> World::add_player(const string& name)
{
	shared_ptr<Player> player = pImpl->file.load_player(name);
	// never nullptr; if the file does not exist, WorldFile makes a new player
	assert(player != nullptr);
	pImpl->players.emplace(name, player);
	return player;
}

shared_ptr<Player> World::get_player(const string& name)
{
	const auto i = pImpl->players.find(name);
	if(i == pImpl->players.cend())
	{
		return nullptr;
	}
	return i->second;
}

const std::unordered_map<string, shared_ptr<Player>>& World::get_players()
{
	return pImpl->players;
}

void World::save()
{
	pImpl->file.save_world();
	pImpl->file.save_players();

	while(!pImpl->chunks_to_save.empty())
	{
		const auto i = pImpl->chunks_to_save.cbegin();
		const ChunkInWorld position = *i;
		pImpl->chunks_to_save.erase(i);
		shared_ptr<Chunk> chunk = get_chunk(position);
		if(chunk != nullptr)
		{
			pImpl->file.save_chunk(*chunk);
		}
	}
}

uint_fast64_t World::get_ticks() const
{
	return ticks;
}

double World::get_time() const
{
	return ticks / 60.0;
}

void World::set_mesher(unique_ptr<Mesher::Base> mesher)
{
	assert(mesher != nullptr);
	this->mesher = std::move(mesher);
	for(auto& p : pImpl->chunks)
	{
		p.second->update();
	}
}

bool World::is_meshing_queued(const shared_ptr<Chunk>& chunk) const
{
	if(chunk == nullptr)
	{
		return false;
	}
	return pImpl->mesh_thread.has(chunk);
}

bool World::is_meshing_queued(const ChunkInWorld& chunk_pos) const
{
	return is_meshing_queued(get_chunk(chunk_pos));
}

void World::impl::update_chunk_neighbors
(
	const ChunkInWorld& chunk_pos,
	const bool thread
)
{
	update_chunk_neighbor(chunk_pos, {-1,  0,  0}, thread);
	update_chunk_neighbor(chunk_pos, {+1,  0,  0}, thread);
	update_chunk_neighbor(chunk_pos, { 0, -1,  0}, thread);
	update_chunk_neighbor(chunk_pos, { 0, +1,  0}, thread);
	update_chunk_neighbor(chunk_pos, { 0,  0, -1}, thread);
	update_chunk_neighbor(chunk_pos, { 0,  0, +1}, thread);
}

void World::impl::update_chunk_neighbors
(
	const ChunkInWorld& chunk_pos,
	const BlockInChunk& pos,
	const bool thread
)
{
	const auto x = pos.x;
	const auto y = pos.y;
	const auto z = pos.z;

	// TODO: check if the neighbor chunk has a block beside this one (to avoid updating when the appearance won't change)
	if(x == 0)
	{
		update_chunk_neighbor(chunk_pos, {-1, 0, 0}, thread);
	}
	else if(x == CHUNK_SIZE - 1)
	{
		update_chunk_neighbor(chunk_pos, {+1, 0, 0}, thread);
	}

	if(y == 0)
	{
		update_chunk_neighbor(chunk_pos, {0, -1, 0}, thread);
	}
	else if(y == CHUNK_SIZE - 1)
	{
		update_chunk_neighbor(chunk_pos, {0, +1, 0}, thread);
	}

	if(z == 0)
	{
		update_chunk_neighbor(chunk_pos, {0, 0, -1}, thread);
	}
	else if(z == CHUNK_SIZE - 1)
	{
		update_chunk_neighbor(chunk_pos, {0, 0, +1}, thread);
	}
}

void World::impl::update_chunk_neighbor
(
	const ChunkInWorld& chunk_pos,
	const ChunkInWorld& offset,
	const bool thread
)
{
	shared_ptr<Chunk> chunk = world.get_chunk(chunk_pos + offset);
	if(chunk != nullptr)
	{
		if(thread)
		{
			mesh_thread.enqueue(chunk);
		}
		else
		{
			chunk->update();
		}
	}
}

static double sum_noise
(
	const glm::dvec2& P,
	const double base_freq,
	const double freq_mul,
	const std::size_t iterations
)
{
	double val = 0;
	double freq = base_freq;
	for(std::size_t i = 0; i < iterations; i++)
	{
		val += std::abs(glm::simplex(P * freq) / freq);
		freq *= freq_mul;
	}
	return val;
}

void World::impl::gen_chunk(shared_ptr<Chunk> chunk) const
{
	assert(chunk != nullptr);

	const ChunkInWorld chunk_pos = chunk->get_position();
	const BlockInWorld min(chunk_pos, {0, 0, 0});
	const BlockInWorld max(chunk_pos, {CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1});

	BlockInWorld block_pos(0, 0, 0);
	for(auto x = min.x; x <= max.x; ++x)
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
			const glm::dvec2 n(-sum_noise(coords, 1, 2.07, 8));

			const double a = n.x * n.y;
			const double b = glm::mod(a, 1.0);
			const auto d = static_cast<uint_fast8_t>(glm::mod(ceil(a), 2.0));
			return (d == 0 ? b : 1 - b) - 1;
		};

		const auto real_max_y = static_cast<BlockInWorld::value_type>(std::round(get_max_y(x, z) * m));
		const auto max_y = max.y <= -m ? max.y : std::min(max.y, real_max_y);

		block_pos.x = x;
		block_pos.z = z;
		for(auto y = min.y; y <= max_y; ++y)
		{
			block_pos.y = y;

			// TODO: investigate performance of using strings here vs caching the IDs
			const string t = y > -m / 2 ? "test_white" : "test_black";
			chunk->set_block(BlockInChunk(block_pos), world.block_registry.get_default(t));
		}
	}
}
