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

namespace block_thingy {

class World
{
public:
	World
	(
		const fs::path& file_path,
		block::BlockRegistry&,
		std::unique_ptr<mesher::Base>
	);
	~World();

	World(World&&) = delete;
	World(const World&) = delete;
	World& operator=(World&&) = delete;
	World& operator=(const World&) = delete;

	const std::shared_ptr<block::Base> get_block(const position::BlockInWorld&) const;
	std::shared_ptr<block::Base> get_block(const position::BlockInWorld&);

	void set_block
	(
		const position::BlockInWorld&,
		std::shared_ptr<block::Base>,
		bool thread = true
	);

	graphics::Color get_blocklight(const position::BlockInWorld&) const;
	void set_blocklight(const position::BlockInWorld&, const graphics::Color&, bool save);
	void update_blocklight(const position::BlockInWorld&, bool save);
	void update_blocklight(const position::BlockInWorld&, const graphics::Color&, bool save);

	std::shared_ptr<Chunk> get_chunk(const position::ChunkInWorld&) const;
	std::shared_ptr<Chunk> get_or_make_chunk(const position::ChunkInWorld&);
	void set_chunk(const position::ChunkInWorld&, std::shared_ptr<Chunk> chunk);

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
	bool is_meshing_queued(const position::ChunkInWorld&) const;

	// for msgpack
	template<typename T> void save(T&) const;
	template<typename T> void load(const T&);

private:
	uint64_t ticks;

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
