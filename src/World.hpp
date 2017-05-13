#pragma once

#include <functional>
#include <memory>
#include <experimental/propagate_const>
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
#include "util/filesystem.hpp"

class World
{
	public:
		World
		(
			const fs::path& file_path,
			Block::BlockRegistry&,
			std::unique_ptr<Mesher::Base>
		);
		~World();

		World(World&&) = delete;
		World(const World&) = delete;
		void operator=(const World&) = delete;

		const Block::Base& get_block(const Position::BlockInWorld&) const;
		Block::Base& get_block(const Position::BlockInWorld&);

		void set_block
		(
			const Position::BlockInWorld&,
			std::unique_ptr<Block::Base>,
			bool thread = true
		);

		Graphics::Color get_light(const Position::BlockInWorld&) const;
		void set_light
		(
			const Position::BlockInWorld&,
			const Graphics::Color&,
			bool save
		);
		void add_light
		(
			const Position::BlockInWorld&,
			const Graphics::Color&,
			bool save
		);
		void process_light_add();
		void sub_light(const Position::BlockInWorld&);
		void update_light_around(const Position::BlockInWorld&);

		std::shared_ptr<Chunk> get_chunk(const Position::ChunkInWorld&) const;
		std::shared_ptr<Chunk> get_or_make_chunk(const Position::ChunkInWorld&);
		void set_chunk(const Position::ChunkInWorld&, std::shared_ptr<Chunk> chunk);

		void step(double delta_time);

		std::shared_ptr<Player> add_player(const std::string& name);
		std::shared_ptr<Player> get_player(const std::string& name);
		const std::unordered_map<std::string, std::shared_ptr<Player>>& get_players();

		void save();

		uint64_t get_ticks() const;
		double get_time() const;

		Block::BlockRegistry& block_registry;

		void set_mesher(std::unique_ptr<Mesher::Base>);
		std::unique_ptr<Mesher::Base> mesher;

		// for msgpack
		template<typename T> void save(T&) const;
		template<typename T> void load(const T&);

	private:
		uint64_t ticks;

		struct impl;
		std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
