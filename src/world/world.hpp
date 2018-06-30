#pragma once

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <stdint.h>
#include <string>
#include <unordered_set>

#include <msgpack/object_fwd_decl.hpp>
#include <msgpack/pack_decl.hpp>

#include "fwd/Player.hpp"
#include "block/block.hpp"
#include "block/manager.hpp"
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
		std::unique_ptr<mesher::base>
	);
	~world();

	world(world&&) = delete;
	world(const world&) = delete;
	world& operator=(world&&) = delete;
	world& operator=(const world&) = delete;

	block_t get_block(const position::block_in_world&) const;

	void set_block
	(
		const position::block_in_world&,
		block_t,
		bool thread = true
	);

	graphics::color get_light(const position::block_in_world&) const;

	graphics::color get_blocklight(const position::block_in_world&) const;
	void set_blocklight(const position::block_in_world&, const graphics::color&);

	graphics::color get_skylight(const position::block_in_world&) const;
	void set_skylight(const position::block_in_world&, const graphics::color&);

	std::shared_ptr<const Chunk> get_chunk(const position::chunk_in_world&) const;
	std::shared_ptr<      Chunk> get_chunk(const position::chunk_in_world&);
	std::shared_ptr<Chunk> get_or_make_chunk(const position::chunk_in_world&);
	void set_chunk(const position::chunk_in_world&, std::shared_ptr<Chunk> chunk, bool set_light);

	/*
	 * Call this every tick to keep a chunk loaded
	 */
	void mark_chunk_active(const position::chunk_in_world&);

	void step(double delta_time);

	std::shared_ptr<Player> add_player(const std::string& name);
	std::shared_ptr<Player> get_player(const std::string& name);
	const std::map<std::string, std::shared_ptr<Player>>& get_players();

	void save_all();

	std::string get_name() const;
	void set_name(const std::string&);

	double get_seed() const;
	void set_seed(double);

	uint64_t get_ticks() const;
	double get_time() const;

	block::manager block_manager;

	void set_mesher(std::unique_ptr<mesher::base>);
	std::unique_ptr<mesher::base> mesher;
	bool is_meshing_queued(const std::shared_ptr<const Chunk>&) const;
	bool is_meshing_queued(const position::chunk_in_world&) const;

	// for msgpack
	void save(msgpack::packer<std::ofstream>&) const;
	void load(const msgpack::object&);

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
