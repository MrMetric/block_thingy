#pragma once

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <stdint.h>
#include <string>
#include <unordered_set>

#include "fwd/Player.hpp"
#include "fwd/block/base.hpp"
#include "fwd/block/BlockRegistry.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "fwd/chunk/Mesher/base.hpp"
#include "fwd/graphics/color.hpp"
#include "fwd/position/block_in_world.hpp"
#include "fwd/position/chunk_in_world.hpp"
#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::world {

class world
{
public:
	world
	(
		const fs::path& dir_path,
		block::BlockRegistry&,
		std::unique_ptr<mesher::base>
	);
	~world();

	world(world&&) = delete;
	world(const world&) = delete;
	world& operator=(world&&) = delete;
	world& operator=(const world&) = delete;

	const std::shared_ptr<block::base> get_block(const position::block_in_world&) const;
	std::shared_ptr<block::base> get_block(const position::block_in_world&);

	void set_block
	(
		const position::block_in_world&,
		std::shared_ptr<block::base>,
		bool thread = true
	);

	graphics::color get_blocklight(const position::block_in_world&) const;
	void set_blocklight(const position::block_in_world&, const graphics::color&, bool save);
	void update_blocklight(const position::block_in_world&, bool save);
	void update_blocklight(const position::block_in_world&, const graphics::color&, bool save);

	std::shared_ptr<Chunk> get_chunk(const position::chunk_in_world&) const;
	std::shared_ptr<Chunk> get_or_make_chunk(const position::chunk_in_world&);
	void set_chunk(const position::chunk_in_world&, std::shared_ptr<Chunk> chunk);

	void step(double delta_time);

	std::shared_ptr<Player> add_player(const std::string& name);
	std::shared_ptr<Player> get_player(const std::string& name);
	const std::map<std::string, std::shared_ptr<Player>>& get_players();

	void save();

	std::string get_name() const;
	void set_name(const std::string&);

	double get_seed() const;
	void set_seed(double);

	uint64_t get_ticks() const;
	double get_time() const;

	block::BlockRegistry& block_registry;

	void set_mesher(std::unique_ptr<mesher::base>);
	std::unique_ptr<mesher::base> mesher;
	bool is_meshing_queued(const std::shared_ptr<Chunk>&) const;
	bool is_meshing_queued(const position::chunk_in_world&) const;

	// for msgpack
	template<typename T> void save(T&) const;
	template<typename T> void load(const T&);

private:
	// for loading
	void set_ticks(uint64_t);

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
