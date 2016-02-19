#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <unordered_map>

#include "Coords.hpp"

class Chunk;
class Block;

using world_map_keyhasher_t = std::function<uint_fast64_t(Position::ChunkInWorld)>;
using world_map_t = std::unordered_map<Position::ChunkInWorld, std::shared_ptr<Chunk>, world_map_keyhasher_t>;

class World
{
	public:
		World();
		virtual ~World();

		Block get_block(const Position::BlockInWorld&) const;
		void set_block(const Position::BlockInWorld&, Block);

		std::shared_ptr<Chunk> get_chunk(const Position::ChunkInWorld&) const;
		std::shared_ptr<Chunk> get_or_make_chunk(const Position::ChunkInWorld&);
		void set_chunk(const Position::ChunkInWorld&, std::shared_ptr<Chunk> chunk);

		void gen_chunk(const Position::ChunkInWorld&);
		void gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max);

	private:
		world_map_t chunks;
		mutable Position::ChunkInWorld last_key;
		mutable std::shared_ptr<Chunk> last_chunk;
		std::minstd_rand random_engine;
};
