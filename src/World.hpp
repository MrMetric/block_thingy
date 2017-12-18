#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "fwd/Player.hpp"
#include "fwd/block/Base.hpp"
#include "fwd/block/BlockRegistry.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "fwd/chunk/Mesher/Base.hpp"
#include "fwd/graphics/Color.hpp"
#include "fwd/position/BlockInWorld.hpp"
#include "fwd/position/ChunkInWorld.hpp"
#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::world {

class world
{
public:
	world
	(
		const fs::path& file_path,
		block::BlockRegistry&,
		std::unique_ptr<mesher::Base>
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
	const std::unordered_map<std::string, std::shared_ptr<Player>>& get_players();

	void save();

	uint64_t get_ticks() const;
	double get_time() const;

	block::BlockRegistry& block_registry;

	void set_mesher(std::unique_ptr<mesher::Base>);
	std::unique_ptr<mesher::Base> mesher;
	bool is_meshing_queued(const std::shared_ptr<Chunk>&) const;
	bool is_meshing_queued(const position::chunk_in_world&) const;

	// for msgpack
	template<typename T> void save(T&) const;
	template<typename T> void load(const T&);

private:
	uint64_t ticks;

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
