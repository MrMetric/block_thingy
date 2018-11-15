/*
 * For information on light propagation, see:
 *   https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
 *   https://www.seedofandromeda.com/blogs/30-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-2
 */
#include "world.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <tuple>
#include <unordered_set>
#include <utility>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>
#include <msgpack.hpp>

#include "Player.hpp"
#include "settings.hpp"
#include "chunk/Chunk.hpp"
#include "chunk/Mesher/base.hpp"
#include "graphics/color.hpp"
#include "position/block_in_chunk.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"
#include "position/hash.hpp"
#include "storage/world_file.hpp"
#include "storage/msgpack/block_manager.hpp"
#include "storage/msgpack/color.hpp"
#include "storage/msgpack/position.hpp"
#include "util/ThreadThingy.hpp"

using std::nullopt;
using std::string;
using std::shared_ptr;
using std::unique_ptr;

namespace block_thingy::world {

using position::block_in_chunk;
using position::block_in_world;
using position::chunk_in_world;

constexpr std::size_t LIGHT_LAYER_BLOCK = 0;
constexpr std::size_t LIGHT_LAYER_SKY   = 1;
constexpr std::size_t LIGHT_LAYER_COUNT = 2;
constexpr double TICKS_PER_SECOND = 60;

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
			if(chunk == nullptr)
			{
				// should not happen
				return;
			}
			chunk->update();
			mesh_thread.dequeue(chunk);
		}, 2),
		skylight_color(8, 8, 8)
	{
	}

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	world& this_world;

	position::unordered_map_t<chunk_in_world, shared_ptr<Chunk>> chunks;
	mutable std::mutex chunks_mutex;

	std::unordered_set<shared_ptr<const Chunk>> chunks_to_save;

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

	std::unordered_set<chunk_in_world, position::hasher_struct<chunk_in_world>> active_chunks;

	util::ThreadThingy<shared_ptr<Chunk>> mesh_thread;

	std::deque<std::tuple<block_in_world, graphics::color>> light_sub1[LIGHT_LAYER_COUNT];
	std::deque<std::tuple<block_in_world, graphics::color>> light_sub2[LIGHT_LAYER_COUNT];

	std::deque<block_in_world> light_add1[LIGHT_LAYER_COUNT];
	std::deque<block_in_world> light_add2[LIGHT_LAYER_COUNT];

	graphics::color skylight_color; // perhaps should be in world instance

	graphics::color get_light(std::size_t layer, const block_in_world&) const;
	void set_light(std::size_t layer, const block_in_world&, const graphics::color&);

	void sub_light(std::size_t layer, const block_in_world&, const graphics::color&);
	void add_light(std::size_t layer, const block_in_world&, const graphics::color&);

	void process_light_sub(std::size_t layer);
	void process_blocklight_sub();
	void process_skylight_sub();

	void process_light_add(std::size_t layer);
	void process_blocklight_add();
	void process_skylight_add();

	void update_light_around(std::size_t layer, const block_in_world&);
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
	pImpl->chunks_to_save.emplace(chunk);

	const bool old_affects_light = does_affect_light(block_manager, old_block);
	const bool affects_light = does_affect_light(block_manager, block);

	const graphics::color old_light = block_manager.info.light(old_block);
	const graphics::color light = block_manager.info.light(block);

	// TODO: these checks might not work (a filter block could be overwritten by a different filter block)
	if(affects_light && !old_affects_light
	|| old_light != light)
	{
		pImpl->sub_light(LIGHT_LAYER_BLOCK, block_pos, chunk->get_blocklight(pos));
	}
	if(old_light != light)
	{
		pImpl->add_light(LIGHT_LAYER_BLOCK, block_pos, light);
	}
	if(affects_light != old_affects_light)
	{
		pImpl->update_light_around(LIGHT_LAYER_BLOCK, block_pos);
		pImpl->update_light_around(LIGHT_LAYER_SKY, block_pos);
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

graphics::color world::get_light(const block_in_world& block_pos) const
{
	const chunk_in_world chunk_pos(block_pos);
	const shared_ptr<const Chunk> chunk = get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		return {0, 0, 0};
	}
	return chunk->get_light(block_in_chunk(block_pos));
}

graphics::color world::get_blocklight(const block_in_world& block_pos) const
{
	return pImpl->get_light(LIGHT_LAYER_BLOCK, block_pos);
}

void world::set_blocklight
(
	const block_in_world& block_pos,
	const graphics::color& color
)
{
	pImpl->set_light(LIGHT_LAYER_BLOCK, block_pos, color);
}

graphics::color world::get_skylight(const block_in_world& block_pos) const
{
	return pImpl->get_light(LIGHT_LAYER_SKY, block_pos);
}

void world::set_skylight
(
	const block_in_world& block_pos,
	const graphics::color& color
)
{
	pImpl->set_light(LIGHT_LAYER_SKY, block_pos, color);
}

graphics::color world::impl::get_light(const std::size_t layer, const block_in_world& block_pos) const
{
	const chunk_in_world chunk_pos(block_pos);
	const shared_ptr<const Chunk> chunk = this_world.get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		return {0, 0, 0};
	}

	const block_in_chunk pos(block_pos);
	if(layer == LIGHT_LAYER_BLOCK)
	{
		return chunk->get_blocklight(pos);
	}
	assert(layer == LIGHT_LAYER_SKY);
	return chunk->get_skylight(pos);
}

void world::impl::set_light
(
	const std::size_t layer,
	const block_in_world& block_pos,
	const graphics::color& color
)
{
	const chunk_in_world chunk_pos(block_pos);
	const shared_ptr<Chunk> chunk = this_world.get_chunk(chunk_pos);
	if(chunk == nullptr)
	{
		// TODO?: handle this better
		return;
	}

	const block_in_chunk pos(block_pos);
	if(layer == LIGHT_LAYER_BLOCK)
	{
		chunk->set_blocklight(pos, color);
	}
	else
	{
		assert(layer == LIGHT_LAYER_SKY);
		chunk->set_skylight(pos, color);
	}

	// update light in neighboring chunks
	{
		glm::tvec3<bool> xyz(false, false, false);
		glm::tvec3<bool> zero(glm::uninitialize);
		const graphics::color color2 = chunk->get_light(pos);
		auto do_it = [this, &chunk_pos, &color=color2, &pos, &zero](const glm::tvec3<bool>& xyz)
		{
			chunk_in_world chunk_pos_2 = chunk_pos;
			if(xyz.x) chunk_pos_2.x += (zero.x ? -1 : 1);
			if(xyz.y) chunk_pos_2.y += (zero.y ? -1 : 1);
			if(xyz.z) chunk_pos_2.z += (zero.z ? -1 : 1);
			const shared_ptr<Chunk> chunk2 = this_world.get_chunk(chunk_pos_2);
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

	chunks_to_save.emplace(chunk);
}

void world::impl::sub_light
(
	const std::size_t layer,
	const block_in_world& block_pos,
	const graphics::color& color
)
{
	if(color == 0)
	{
		return;
	}

	light_sub1[layer].emplace_back(block_pos, color);
}

void world::impl::add_light
(
	const std::size_t layer,
	const block_in_world& block_pos,
	const graphics::color& color
)
{
	// TODO: should it add to the current value, not set it?
	set_light(layer, block_pos, color);
	if(color == 0)
	{
		return;
	}

	light_add1[layer].emplace_back(block_pos);
}

void world::impl::process_light_add(const std::size_t layer)
{
	if(layer == LIGHT_LAYER_BLOCK)
	{
		process_blocklight_add();
	}
	else
	{
		assert(layer == LIGHT_LAYER_SKY);
		process_skylight_add();
	}
}

void world::impl::process_blocklight_add()
{
	auto& light_add1 = this->light_add1[LIGHT_LAYER_BLOCK];
	auto& light_add2 = this->light_add2[LIGHT_LAYER_BLOCK];
	while(!light_add1.empty())
	{
		const block_in_world pos = light_add1.front();
		light_add1.pop_front();

		// bad solution, but works for now
		if(this_world.get_chunk(chunk_in_world(pos)) == nullptr)
		{
			light_add2.emplace_back(pos);
			continue;
		}

		const graphics::color color = this_world.get_blocklight(pos) - 1;
		if(color == 0)
		{
			continue;
		}

		auto fill =
		[
			this,
			&light_add2,
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
			const chunk_in_world cpos2(pos2);
			const shared_ptr<const Chunk> chunk = this_world.get_chunk(cpos2);
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
				this_world.set_blocklight(pos2, color2);
				light_add2.emplace_back(pos2);
			}
		};

		fill( 0,  0, -1, color);
		fill( 0,  0, +1, color);
		fill( 0, -1,  0, color);
		fill( 0, +1,  0, color);
		fill(-1,  0,  0, color);
		fill(+1,  0,  0, color);
	}
	std::swap(light_add1, light_add2);
}

void world::impl::process_skylight_add()
{
	auto& light_add1 = this->light_add1[LIGHT_LAYER_SKY];
	auto& light_add2 = this->light_add2[LIGHT_LAYER_SKY];
	while(!light_add1.empty())
	{
		const block_in_world pos = light_add1.front();
		light_add1.pop_front();

		// bad solution, but works for now
		if(this_world.get_chunk(chunk_in_world(pos)) == nullptr)
		{
			light_add2.emplace_back(pos);
			continue;
		}

		const graphics::color color = this_world.get_skylight(pos);
		const graphics::color color1 = color - 1;

		auto fill =
		[
			this,
			&light_add2,
			&pos
		]
		(
			const int8_t x,
			const int8_t y,
			const int8_t z,
			graphics::color color
		)
		{
			if(color == 0)
			{
				return;
			}
			const block_in_world pos2{pos.x + x, pos.y + y, pos.z + z};
			const chunk_in_world cpos2(pos2);
			const shared_ptr<const Chunk> chunk = this_world.get_chunk(cpos2);
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
			graphics::color color2 = chunk->get_skylight(pos2b);
			bool set = false;
			if(color2.r < color.r) { color2.r = color.r; set = true; }
			if(color2.g < color.g) { color2.g = color.g; set = true; }
			if(color2.b < color.b) { color2.b = color.b; set = true; }
			if(set)
			{
				this_world.set_skylight(pos2, color2);
				light_add2.emplace_back(pos2);
			}
		};

		fill( 0,  0, -1, color1);
		fill( 0,  0, +1, color1);

		/*
		Despite their drawbacks, either of these methods is reasonable. I chose the simpler one.
		I could make it work for filtered light by storing a bool with the color.
		The bool indicates whether or not the light is in its originating column.
		If true, the sky visibility check applies. If false, it always dims going down.
		*/

		// checking if the sky is visible causes a problem when there is a filter block that sees the sky and brighter light goes underneath it horizontally
		//const shared_ptr<const Chunk> chunk = this_world.get_chunk(chunk_in_world(pos));
		//if(chunk.can_see_sky_at(pos))

		// comparing the color causes light to start dimming when skylight goes through a light filter
		if(color == skylight_color)
		{
			fill( 0, -1,  0, color);
		}
		else
		{
			fill( 0, -1,  0, color1);
		}

		fill( 0, +1,  0, color1);
		fill(-1,  0,  0, color1);
		fill(+1,  0,  0, color1);
	}
	std::swap(light_add1, light_add2);
}

void world::impl::process_light_sub(const std::size_t layer)
{
	if(layer == LIGHT_LAYER_BLOCK)
	{
		process_blocklight_sub();
	}
	else
	{
		assert(layer == LIGHT_LAYER_SKY);
		process_skylight_sub();
	}
}

void world::impl::process_blocklight_sub()
{
	auto& light_add = this->light_add1[LIGHT_LAYER_BLOCK];
	auto& light_sub = this->light_sub1[LIGHT_LAYER_BLOCK];
	auto& light_sub2 = this->light_sub2[LIGHT_LAYER_BLOCK];

	while(!light_sub.empty())
	{
		const auto front = light_sub.front();
		const block_in_world pos = std::get<0>(front);
		const graphics::color color = std::get<1>(front);
		light_sub.pop_front();
		if(color == 0)
		{
			continue;
		}

		// bad solution, but works for now
		if(this_world.get_chunk(chunk_in_world(pos)) == nullptr)
		{
			light_sub2.emplace_back(front);
			continue;
		}

		auto fill =
		[
			this,
			&light_add,
			&light_sub2,
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
			const shared_ptr<const Chunk> chunk = this_world.get_chunk(chunk_in_world(pos2));
			if(chunk == nullptr)
			{
				return;
			}
			const block_in_chunk pos2b(pos2);
			const block_t block = chunk->get_block(pos2b);
			const bool is_source = this_world.block_manager.info.light(block) != 0;
			graphics::color color2 = chunk->get_blocklight(pos2b);
			graphics::color color_set = color2;
			graphics::color color_put(0, 0, 0);

			bool set = false;
			for(uint_fast8_t i = 0; i < 3; ++i)
			{
				if(!is_source && color2[i] != 0 && color2[i] < color[i])
				{
					color_set[i] = 0;
					color_put[i] = color2[i];
					set = true;
				}
				else if(color2[i] >= color[i])
				{
					light_add.emplace_back(pos2);
				}
			}
			if(set)
			{
				this_world.set_blocklight(pos2, color_set);
				light_sub2.emplace_back(pos2, color_put);
			}
		};

		fill(color,  0,  0, -1);
		fill(color,  0,  0, +1);
		fill(color,  0, -1,  0);
		fill(color,  0, +1,  0);
		fill(color, -1,  0,  0);
		fill(color, +1,  0,  0);
	}

	std::swap(light_sub, light_sub2);
}
void world::impl::process_skylight_sub()
{
	auto& light_add = this->light_add1[LIGHT_LAYER_SKY];
	auto& light_sub = this->light_sub1[LIGHT_LAYER_SKY];
	auto& light_sub2 = this->light_sub2[LIGHT_LAYER_SKY];

	while(!light_sub.empty())
	{
		const auto front = light_sub.front();
		const block_in_world pos = std::get<0>(front);
		const graphics::color color = std::get<1>(front);
		light_sub.pop_front();
		if(color == 0)
		{
			continue;
		}

		// bad solution, but works for now
		if(this_world.get_chunk(chunk_in_world(pos)) == nullptr)
		{
			light_sub2.emplace_back(front);
			continue;
		}

		this_world.set_skylight(pos, {0, 0, 0});

		auto fill =
		[
			this,
			&light_add,
			&light_sub2,
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
			const shared_ptr<const Chunk> chunk = this_world.get_chunk(chunk_in_world(pos2));
			if(chunk == nullptr)
			{
				return;
			}
			const block_in_chunk pos2b(pos2);
			const block_t block = chunk->get_block(pos2b);
			const bool is_source = this_world.block_manager.info.light(block) != 0;
			graphics::color color2 = chunk->get_skylight(pos2b);
			graphics::color color_set = color2;
			graphics::color color_put(0, 0, 0);

			bool set = false;
			if(y == -1)
			{
				for(uint_fast8_t i = 0; i < 3; ++i)
				{
					if(!is_source && color2[i] != 0 && color2[i] <= color[i])
					{
						color_set[i] = 0;
						color_put[i] = color2[i];
						set = true;
					}
					else if(color2[i] > color[i])
					{
						light_add.emplace_back(pos2);
					}
				}
			}
			else
			{
				for(uint_fast8_t i = 0; i < 3; ++i)
				{
					if(!is_source && color2[i] != 0 && color2[i] < color[i])
					{
						color_set[i] = 0;
						color_put[i] = color2[i];
						set = true;
					}
					else if(color2[i] >= color[i])
					{
						light_add.emplace_back(pos2);
					}
				}
			}
			if(set)
			{
				this_world.set_skylight(pos2, color_set);
				light_sub2.emplace_back(pos2, color_put);
			}
		};

		fill(color,  0,  0, -1);
		fill(color,  0,  0, +1);
		fill(color,  0, -1,  0);
		fill(color,  0, +1,  0);
		fill(color, -1,  0,  0);
		fill(color, +1,  0,  0);
	}

	std::swap(light_sub, light_sub2);
}

void world::impl::update_light_around(const std::size_t layer, const block_in_world& block_pos)
{
	auto& light_add = this->light_add1[layer];
	#define a(x_, y_, z_) light_add.emplace_back(block_pos.x + (x_), block_pos.y + (y_), block_pos.z + (z_))
	a( 0,  0, -1);
	a( 0,  0, +1);
	a( 0, -1,  0);
	a( 0, +1,  0);
	a(-1,  0,  0);
	a(+1,  0,  0);
	#undef a
}

void world::set_chunk
(
	const chunk_in_world& chunk_pos,
	shared_ptr<Chunk> chunk,
	const bool set_light
)
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

	// set skylight
	// TODO: handle skylight being blocked by above chunks
	// currently, every chunk has skylight emitted at the top
	if(set_light)
	{
		auto& skylight_add = pImpl->light_add1[LIGHT_LAYER_SKY];
		block_in_chunk pos(0, CHUNK_SIZE - 1, 0);
		for(pos.x = 0; pos.x < CHUNK_SIZE; ++pos.x)
		for(pos.z = 0; pos.z < CHUNK_SIZE; ++pos.z)
		{
			graphics::color light = pImpl->skylight_color;
			const block_t block = chunk->get_block(pos);
			if(block_manager.info.is_opaque(block))
			{
				light = 0;
			}
			else if(block_manager.info.is_translucent(block))
			{
				const graphics::color f = block_manager.info.light_filter(block);
				light.r = std::min(light.r, f.r);
				light.g = std::min(light.g, f.g);
				light.b = std::min(light.b, f.b);
			}
			chunk->set_skylight(pos, light);
			const block_in_world block_pos(chunk->get_position(), pos);
			skylight_add.emplace_back(block_pos);
		}
	}

	// update light at chunk sides to make it flow into the new chunk
	/*
	if(set_light)
	{
		auto& blocklight_add = pImpl->light_add1[LIGHT_LAYER_BLOCK];
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
				blocklight_add.emplace_back(pos2);

				pos2[i3] = pos1[i3] + CHUNK_SIZE;
				blocklight_add.emplace_back(pos2);
			}
		}
	}
	*/

	// update blocklight
	if(set_light)
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
				pImpl->add_light(LIGHT_LAYER_BLOCK, {chunk_pos, pos}, light);
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
			block_in_chunk lpos2;
			glm::ivec3 pos1;
			block_in_chunk lpos1;
			for(uint_fast8_t i = 0; i < 3; ++i)
			{
				if(pos2[i] == -1)
				{
					chunk_pos_2[i] -= 1;
					lpos2[i] = CHUNK_SIZE - 1;
					pos1[i] = CHUNK_SIZE;
					lpos1[i] = 0;
				}
				else if(pos2[i] == CHUNK_SIZE)
				{
					chunk_pos_2[i] += 1;
					lpos2[i] = 0;
					pos1[i] = -1;
					lpos1[i] = CHUNK_SIZE - 1;
				}
				else
				{
					lpos2[i] = static_cast<block_in_chunk::value_type>(pos2[i]);
					pos1[i] = pos2[i];
					lpos1[i] = static_cast<block_in_chunk::value_type>(pos2[i]);
				}
			}
			if(chunk_pos_2 == chunk_pos)
			{
				continue;
			}
			const shared_ptr<Chunk> chunk2 = get_chunk(chunk_pos_2);
			if(chunk2 != nullptr)
			{
				chunk->set_texbuflight(pos2, chunk2->get_light(lpos2));
				chunk2->set_texbuflight(pos1, chunk->get_light(lpos1));
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

void world::mark_chunk_active(const chunk_in_world& chunk_pos)
{
	pImpl->active_chunks.emplace(chunk_pos);
}

void world::step()
{
	shared_ptr<Chunk> chunk;
	if(pImpl->loaded_chunks.try_dequeue(chunk))
	{
		chunk_in_world pos = chunk->get_position();
		set_chunk(pos, chunk, false);
		pImpl->load_thread.dequeue(pos);
	}
	if(pImpl->generated_chunks.try_dequeue(chunk))
	{
		chunk_in_world pos = chunk->get_position();
		set_chunk(pos, chunk, true);
		pImpl->gen_thread.dequeue(pos);
		pImpl->chunks_to_save.emplace(chunk);
	}
	if(!pImpl->chunks_to_save.empty())
	{
		const auto i = pImpl->chunks_to_save.cbegin();
		const shared_ptr<const Chunk> chunk = *i;
		pImpl->chunks_to_save.erase(i);
		if(chunk != nullptr)
		{
			pImpl->file.save_chunk(*chunk);
		}
		else
		{
			// should not happen
		}
	}

	pImpl->active_chunks.clear();

	pImpl->process_light_sub(LIGHT_LAYER_BLOCK);
	pImpl->process_light_sub(LIGHT_LAYER_SKY);

	pImpl->process_light_add(LIGHT_LAYER_BLOCK);
	pImpl->process_light_add(LIGHT_LAYER_SKY);

	const auto render_distance = static_cast<uint64_t>(settings::get<int64_t>("render_distance"));
	for(auto& [name, player] : pImpl->players)
	{
		player->step(*this);

		const chunk_in_world chunk_pos = player->view_position_chunk();
		const chunk_in_world::value_type render_distance_ = static_cast<chunk_in_world::value_type>(render_distance);
		const chunk_in_world min = chunk_pos - render_distance_;
		const chunk_in_world max = chunk_pos + render_distance_;

		chunk_in_world pos;
		for(pos.x = min.x; pos.x <= max.x; ++pos.x)
		for(pos.y = min.y; pos.y <= max.y; ++pos.y)
		for(pos.z = min.z; pos.z <= max.z; ++pos.z)
		{
			mark_chunk_active(pos);
		}
	}

	{
		std::lock_guard<std::mutex> g(pImpl->chunks_mutex);
		auto prev_chunks = std::move(pImpl->chunks);
		for(const auto& [chunk_pos, chunk] : prev_chunks)
		{
			if(const auto i = pImpl->active_chunks.find(chunk_pos);
				i != pImpl->active_chunks.cend())
			{
				pImpl->chunks.emplace(chunk_pos, std::move(chunk));
				continue;
			}

			// no need to add the chunk to pImpl->chunks_to_save
			// if the chunk needs to be saved, it will already be there
		}
	}
	for(const auto& chunk_pos : pImpl->active_chunks)
	{
		get_or_make_chunk(chunk_pos);
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

void world::save_all()
{
	pImpl->file.save_world(*this);
	for(const auto& [name, player] : pImpl->players)
	{
		pImpl->file.save_player(*player);
	}

	while(!pImpl->chunks_to_save.empty())
	{
		const auto i = pImpl->chunks_to_save.cbegin();
		const shared_ptr<const Chunk> chunk = *i;
		pImpl->chunks_to_save.erase(i);
		if(chunk == nullptr)
		{
			// should not happen
			continue;
		}
		pImpl->file.save_chunk(*chunk);
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
	return pImpl->ticks / TICKS_PER_SECOND;
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

void world::save(msgpack::packer<std::ofstream>& o) const
{
	o.pack_array(8);
	o.pack(pImpl->name);
	o.pack(pImpl->seed);
	o.pack(pImpl->ticks);
	o.pack(block_manager);
	o.pack(pImpl->light_sub1);
	o.pack(pImpl->light_sub2);
	o.pack(pImpl->light_add1);
	o.pack(pImpl->light_add2);
}

template<typename T, std::size_t N>
void load_deques(std::deque<T>(&d)[N], const msgpack::object& o)
{
	auto v = o.as<std::vector<std::deque<T>>>();
	if(v.size() != N) throw msgpack::type_error();
	for(std::size_t i = 0; i < v.size(); ++i)
	{
		d[i] = std::move(v[i]);
	}
}
void world::load(const msgpack::object& o)
{
	if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
	if(o.via.array.size != 8) throw msgpack::type_error();
	const auto& a = o.via.array.ptr;

	pImpl->name = a[0].as<string>();
	pImpl->seed = a[1].as<double>();
	pImpl->ticks = a[2].as<uint64_t>();
	block_manager.load(a[3]);

	load_deques(pImpl->light_sub1, a[4]);
	load_deques(pImpl->light_sub2, a[5]);
	load_deques(pImpl->light_add1, a[6]);
	load_deques(pImpl->light_add2, a[7]);
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
