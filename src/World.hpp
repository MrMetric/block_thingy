#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <unordered_map>

#include "Coords.hpp"

class Chunk;
class Block;

using map = std::unordered_map<Position::ChunkInWorld, std::shared_ptr<Chunk>, std::function<uint_fast64_t(Position::ChunkInWorld)>>;

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
		map chunks;
		mutable Position::ChunkInWorld last_key;
		mutable std::shared_ptr<Chunk> last_chunk;
		std::minstd_rand random_engine;
};
