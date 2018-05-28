#include "world.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <tuple>
#include <unordered_set>
#include <utility>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>

#include "Player.hpp"
#include "chunk/Chunk.hpp"
#include "chunk/Mesher/base.hpp"
#include "graphics/color.hpp"
#include "position/block_in_chunk.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"
#include "position/hash.hpp"
#include "storage/world_file.hpp"
#include "util/ThreadThingy.hpp"

using std::nullopt;
using std::string;
using std::shared_ptr;
using std::unique_ptr;

namespace block_thingy::world {

using position::block_in_chunk;
using position::block_in_world;
using position::chunk_in_world;

struct world::impl
{
	impl
	(
		world& world,
		const fs::path& dir_path
	)
	:
		this_world(world),
		file(dir_path),
		seed(0),
		ticks(0),
		gen_thread([this, &world](const chunk_in_world& pos)
		{
			shared_ptr<Chunk> chunk = std::make_shared<Chunk>(pos, world);
			gen_chunk(chunk);
			generated_chunks.enqueue(chunk);
		}, 2, position::hasher<chunk_in_world>),
		load_thread([this](const chunk_in_world& pos)
		{
			shared_ptr<Chunk> chunk(file.load_chunk(this_world, pos));
			assert(chunk != nullptr);
			loaded_chunks.enqueue(chunk);
			mesh_thread.enqueue(chunk);
		}, 2, position::hasher<chunk_in_world>),
		mesh_thread([this](shared_ptr<Chunk>& chunk)
		{
			assert(chunk != nullptr);
			chunk->update();
			mesh_thread.dequeue(chunk);
		}, 2)
	{
	}

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	world& this_world;

	position::unordered_map_t<chunk_in_world, shared_ptr<Chunk>> chunks;
	mutable std::mutex chunks_mutex;

	std::unordered_set<chunk_in_world, position::hasher_struct<chunk_in_world>> chunks_to_save;

	std::map<string, shared_ptr<Player>> players;

	storage::world_file file;
	string name;
	double seed;
	uint64_t ticks;

	void update_chunk_neighbors
	(
		const chunk_in_world&,
		bool thread = true
	);
	void update_chunk_neighbors
	(
		const chunk_in_world&,
		const block_in_chunk&,
		bool thread = true
	);
	void update_chunk_neighbor
	(
		const chunk_in_world&,
		const chunk_in_world&,
		bool thread = true
	);

	util::ThreadThingy<chunk_in_world, position::hasher_t<chunk_in_world>> gen_thread;
	moodycamel::ConcurrentQueue<shared_ptr<Chunk>> generated_chunks;
	void gen_chunk(shared_ptr<Chunk>&) const;

	util::ThreadThingy<chunk_in_world, position::hasher_t<chunk_in_world>> load_thread;
	moodycamel::ConcurrentQueue<shared_ptr<Chunk>> loaded_chunks;

	util::ThreadThingy<shared_ptr<Chunk>> mesh_thread;

	std::queue<block_in_world> blocklight_add;
	void add_blocklight(const block_in_world&, const graphics::color&, bool save);
	void process_blocklight_add();
	void sub_blocklight(const block_in_world&);
	void update_blocklight_around(const block_in_world&);
};

world::world
(
	const fs::path& dir_path,
	unique_ptr<mesher::base> mesher
)
:
	mesher(std::move(mesher)),
	pImpl(std::make_unique<impl>
	(
		*this,
		dir_path
	))
{
	pImpl->file.load(*this);
}

world::~world()
{
	// this does not work in ~impl
	pImpl->gen_thread.stop();
	pImpl->load_thread.stop();
	pImpl->mesh_thread.stop();
}

static bool does_affect_light(const block::manager& block_manager, const block_t block)
{
	if(block_manager.info.is_opaque(block))
	{
		return true;
	}
	static graphics::color max(graphics::color::max);
	if(block_manager.info.is_translucent(block)
	&& block_manager.info.light_filter(block) != max)
	{
		return true;
	}
	return false;
}

void world::set_block
(
	const block_in_world& block_pos,
	const block_t block,
	bool thread
)
{
	const chunk_in_world chunk_pos(block_pos);
	shared_ptr<Chunk> chunk = get_or_make_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		// TODO: handle this better
		return;
	}

	const block_t old_block = get_block(block_pos);
	if(old_block == block)
	{
		return;
	}

	const block_in_chunk pos(block_pos);
	chunk->set_block(pos, block);
	pImpl->chunks_to_save.emplace(chunk_pos);

	const bool old_affects_light = does_affect_light(block_manager, old_block);
	const bool affects_light = does_affect_light(block_manager, block);

	const graphics::color old_light = block_manager.info.light(old_block);
	const graphics::color light = block_manager.info.light(block);

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

block_t world::get_block(const block_in_world& block_pos) const
{
	const chunk_in_world chunk_pos(block_pos);
	const shared_ptr<const Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		return {};
	}

	block_in_chunk pos(block_pos);
	return chunk->get_block(pos);
}

graphics::color world::get_blocklight(const block_in_world& block_pos) const
{
	const chunk_in_world chunk_pos(block_pos);
	const shared_ptr<const Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		return {0, 0, 0};
	}
	return chunk->get_blocklight(block_in_chunk(block_pos));
}

void world::set_blocklight
(
	const block_in_world& block_pos,
	const graphics::color& color,
	bool save
)
{
	const chunk_in_world chunk_pos(block_pos);
	const shared_ptr<Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		// TODO?: handle this better
		return;
	}
	const block_in_chunk pos(block_pos);
	chunk->set_blocklight(pos, color);

	// update light in neighboring chunks
	{
		glm::tvec3<bool> xyz(false, false, false);
		glm::tvec3<bool> zero(glm::uninitialize);
		auto do_it = [this, &chunk_pos, &color, &pos, &zero](const glm::tvec3<bool>& xyz)
		{
			chunk_in_world chunk_pos_2 = chunk_pos;
			if(xyz.x) chunk_pos_2.x += (zero.x ? -1 : 1);
			if(xyz.y) chunk_pos_2.y += (zero.y ? -1 : 1);
			if(xyz.z) chunk_pos_2.z += (zero.z ? -1 : 1);
			const shared_ptr<Chunk> chunk2 = get_chunk(chunk_pos_2);
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

void world::update_blocklight
(
	const block_in_world& block_pos,
	const bool save
)
{
	update_blocklight(block_pos, block_manager.info.light(get_block(block_pos)), save);
}

void world::update_blocklight
(
	const block_in_world& block_pos,
	const graphics::color& color,
	const bool save
)
{
	pImpl->sub_blocklight(block_pos);
	pImpl->add_blocklight(block_pos, color, save);
}

void world::impl::add_blocklight
(
	const block_in_world& block_pos,
	const graphics::color& color,
	const bool save
)
{
	this_world.set_blocklight(block_pos, color, save);
	if(color == 0)
	{
		return;
	}

	// see https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
	blocklight_add.emplace(block_pos);
	process_blocklight_add();
}

void world::impl::process_blocklight_add()
{
	while(!blocklight_add.empty())
	{
		const block_in_world pos = blocklight_add.front();
		blocklight_add.pop();
		const graphics::color color = this_world.get_blocklight(pos) - 1;
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
			graphics::color color
		)
		{
			const block_in_world pos2{pos.x + x, pos.y + y, pos.z + z};
			const shared_ptr<const Chunk> chunk = this_world.get_chunk(chunk_in_world(pos2));
			if(chunk == nullptr)
			{
				return;
			}
			const block_in_chunk pos2b(pos2);
			const block_t block = chunk->get_block(pos2b);
			if(this_world.block_manager.info.is_opaque(block))
			{
				return;
			}
			if(this_world.block_manager.info.is_translucent(block))
			{
				const graphics::color f = this_world.block_manager.info.light_filter(block);
				color.r = std::min(color.r, f.r);
				color.g = std::min(color.g, f.g);
				color.b = std::min(color.b, f.b);
			}
			graphics::color color2 = chunk->get_blocklight(pos2b);
			bool set = false;
			if(color2.r < color.r) { color2.r = color.r; set = true; }
			if(color2.g < color.g) { color2.g = color.g; set = true; }
			if(color2.b < color.b) { color2.b = color.b; set = true; }
			if(set)
			{
				this_world.set_blocklight(pos2, color2, false);
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

void world::impl::sub_blocklight(const block_in_world& block_pos)
{
	const graphics::color color = this_world.get_blocklight(block_pos);
	if(color == 0)
	{
		return;
	}
	this_world.set_blocklight(block_pos, {0, 0, 0}, false);

	std::queue<std::tuple<block_in_world, graphics::color>> q;
	q.emplace(block_pos, color);
	while(!q.empty())
	{
		const block_in_world pos = std::get<0>(q.front());
		const graphics::color color = std::get<1>(q.front());
		q.pop();

		auto fill =
		[
			this,
			&q,
			&pos
		]
		(
			const graphics::color& color,
			const int8_t x,
			const int8_t y,
			const int8_t z
		)
		{
			const block_in_world pos2{pos.x + x, pos.y + y, pos.z + z};
			graphics::color color2 = this_world.get_blocklight(pos2);
			graphics::color color_set = color2;
			graphics::color color_put(0, 0, 0);

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
				this_world.set_blocklight(pos2, color_set, false);
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

void world::impl::update_blocklight_around(const block_in_world& block_pos)
{
	#define a(x_, y_, z_) blocklight_add.emplace(block_pos.x + (x_), block_pos.y + (y_), block_pos.z + (z_))
	a( 0,  0, -1);
	a( 0,  0, +1);
	a( 0, -1,  0);
	a( 0, +1,  0);
	a(-1,  0,  0);
	a(+1,  0,  0);
	#undef a
	process_blocklight_add();
}

void world::set_chunk(const chunk_in_world& chunk_pos, shared_ptr<Chunk> chunk)
{
	const shared_ptr<const Chunk> prev_chunk = get_chunk(chunk_pos);
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
		block_in_world pos1(chunk_pos, {0, 0, 0});
		block_in_world pos;
		for(uint_fast8_t i1 = 0; i1 < 3; ++i1)
		{
			uint_fast8_t i2 = (i1 + 1) % 3;
			uint_fast8_t i3 = (i1 + 2) % 3;
			for(pos[i1] = pos1[i1]; pos[i1] < pos1[i1] + CHUNK_SIZE; ++pos[i1])
			for(pos[i2] = pos1[i2]; pos[i2] < pos1[i2] + CHUNK_SIZE; ++pos[i2])
			{
				block_in_world pos2(pos);

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
		block_in_chunk pos;
		for(pos.x = 0; pos.x < CHUNK_SIZE; ++pos.x)
		for(pos.y = 0; pos.y < CHUNK_SIZE; ++pos.y)
		for(pos.z = 0; pos.z < CHUNK_SIZE; ++pos.z)
		{
			const block_t block = chunk->get_block(pos);
			const graphics::color light = block_manager.info.light(block);
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
			chunk_in_world chunk_pos_2 = chunk_pos;
			block_in_chunk pos;
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
					pos[i] = static_cast<block_in_chunk::value_type>(pos2[i]);
				}
			}
			if(chunk_pos_2 == chunk_pos)
			{
				continue;
			}
			const shared_ptr<Chunk> chunk2 = get_chunk(chunk_pos_2);
			if(chunk2 != nullptr)
			{
				chunk->set_texbuflight(pos2, chunk2->get_blocklight(pos));
			}
		}
	}

	pImpl->mesh_thread.enqueue(chunk);
	pImpl->update_chunk_neighbors(chunk_pos);
}

shared_ptr<const Chunk> world::get_chunk(const chunk_in_world& chunk_pos) const
{
	std::lock_guard<std::mutex> g(pImpl->chunks_mutex);
	const auto i = pImpl->chunks.find(chunk_pos);
	if(i == pImpl->chunks.cend())
	{
		return nullptr;
	}
	return i->second;
}

shared_ptr<Chunk> world::get_chunk(const chunk_in_world& chunk_pos)
{
	std::lock_guard<std::mutex> g(pImpl->chunks_mutex);
	const auto i = pImpl->chunks.find(chunk_pos);
	if(i == pImpl->chunks.cend())
	{
		return nullptr;
	}
	return i->second;
}

shared_ptr<Chunk> world::get_or_make_chunk(const chunk_in_world& chunk_pos)
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

void world::step(double delta_time)
{
	delta_time = 1.0 / 60.0; // TODO

	shared_ptr<Chunk> chunk;
	while(pImpl->loaded_chunks.try_dequeue(chunk))
	{
		chunk_in_world pos = chunk->get_position();
		set_chunk(pos, chunk);
		pImpl->load_thread.dequeue(pos);
	}
	while(pImpl->generated_chunks.try_dequeue(chunk))
	{
		chunk_in_world pos = chunk->get_position();
		set_chunk(pos, chunk);
		pImpl->gen_thread.dequeue(pos);
		pImpl->chunks_to_save.emplace(pos);
	}

	for(auto& p : pImpl->players)
	{
		p.second->step(*this, delta_time);
	}

	pImpl->ticks += 1;
}

shared_ptr<Player> world::add_player(const string& name)
{
	shared_ptr<Player> player = pImpl->file.load_player(name);
	// never nullptr; if the file does not exist, world_file makes a new player
	assert(player != nullptr);
	pImpl->players.emplace(name, player);
	return player;
}

shared_ptr<Player> world::get_player(const string& name)
{
	const auto i = pImpl->players.find(name);
	if(i == pImpl->players.cend())
	{
		return nullptr;
	}
	return i->second;
}

const std::map<string, shared_ptr<Player>>& world::get_players()
{
	return pImpl->players;
}

void world::save()
{
	pImpl->file.save_world(*this);
	for(const auto& [name, player] : pImpl->players)
	{
		pImpl->file.save_player(*player);
	}

	while(!pImpl->chunks_to_save.empty())
	{
		const auto i = pImpl->chunks_to_save.cbegin();
		const chunk_in_world position = *i;
		pImpl->chunks_to_save.erase(i);
		const shared_ptr<const Chunk> chunk = get_chunk(position);
		if(chunk != nullptr)
		{
			pImpl->file.save_chunk(*chunk);
		}
	}
}

string world::get_name() const
{
	return pImpl->name;
}
void world::set_name(const string& name)
{
	pImpl->name = name;
}

double world::get_seed() const
{
	return pImpl->seed;
}
void world::set_seed(const double seed)
{
	pImpl->seed = seed;
}

uint_fast64_t world::get_ticks() const
{
	return pImpl->ticks;
}

double world::get_time() const
{
	return pImpl->ticks / 60.0;
}

void world::set_mesher(unique_ptr<mesher::base> mesher)
{
	assert(mesher != nullptr);
	this->mesher = std::move(mesher);
	for(auto& p : pImpl->chunks)
	{
		p.second->update();
	}
}

bool world::is_meshing_queued(const shared_ptr<const Chunk>& chunk) const
{
	if(chunk == nullptr)
	{
		return false;
	}
	return pImpl->mesh_thread.has(chunk);
}

bool world::is_meshing_queued(const chunk_in_world& chunk_pos) const
{
	return is_meshing_queued(get_chunk(chunk_pos));
}

void world::set_ticks(const uint64_t ticks)
{
	pImpl->ticks = ticks;
}

void world::impl::update_chunk_neighbors
(
	const chunk_in_world& chunk_pos,
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

void world::impl::update_chunk_neighbors
(
	const chunk_in_world& chunk_pos,
	const block_in_chunk& pos,
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

void world::impl::update_chunk_neighbor
(
	const chunk_in_world& chunk_pos,
	const chunk_in_world& offset,
	const bool thread
)
{
	const shared_ptr<Chunk> chunk = this_world.get_chunk(chunk_pos + offset);
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
	const double seed,
	const glm::dvec2& P_,
	const double base_freq,
	const double freq_mul,
	const std::size_t iterations
)
{
	const glm::dvec3 P(P_, seed);
	double val = 0;
	double freq = base_freq;
	for(std::size_t i = 0; i < iterations; i++)
	{
		val += std::abs(glm::simplex(P * freq) / freq);
		freq *= freq_mul;
	}
	return val;
}

void world::impl::gen_chunk(shared_ptr<Chunk>& chunk) const
{
	assert(chunk != nullptr);

	const chunk_in_world chunk_pos = chunk->get_position();
	const block_in_world min(chunk_pos, {0, 0, 0});
	const block_in_world max(chunk_pos, {CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1});

	block_in_world block_pos(0, 0, 0);
	for(auto x = min.x; x <= max.x; ++x)
	for(auto z = min.z; z <= max.z; ++z)
	{
		const double m = 20;
		if(min.y > 0)
		{
			continue;
		}

		// https://www.shadertoy.com/view/Xl3GWS
		auto get_max_y = [seed=this->seed](const block_in_world::value_type x, const block_in_world::value_type z) -> double
		{
			// coords must not be (0, 0) (it makes this function always return 0)
			const glm::dvec2 coords = (x == 0 && z == 0) ? glm::dvec2(0.0001) : glm::dvec2(x, z) / 1024.0;
			const glm::dvec2 n(-sum_noise(seed, coords, 1, 2.07, 8));

			const double a = n.x * n.y;
			const double b = glm::mod(a, 1.0);
			const auto d = static_cast<uint_fast8_t>(glm::mod(ceil(a), 2.0));
			return (d == 0 ? b : 1 - b) - 1;
		};

		const auto real_max_y = static_cast<block_in_world::value_type>(std::round(get_max_y(x, z) * m));
		const auto max_y = max.y <= -m ? max.y : std::min(max.y, real_max_y);

		block_pos.x = x;
		block_pos.z = z;
		for(auto y = min.y; y <= max_y; ++y)
		{
			block_pos.y = y;

			// TODO: investigate performance of using strings here vs caching the IDs
			const string t = y > -m / 2 ? "test_white" : "test_black";
			const auto block = this_world.block_manager.get_block(t);
			if(block != nullopt)
			{
				chunk->set_block(block_in_chunk(block_pos), *block);
			}
			else
			{
				// TODO?
			}
		}
	}
}

}
