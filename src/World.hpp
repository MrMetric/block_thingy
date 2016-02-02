#pragma once

#include <cstdint>
#include <memory>
#include <random>

#include <boost/unordered/unordered_map.hpp>

#include "Coords.hpp"

class Chunk;
class Block;

using map = boost::unordered_map<uint64_t, std::shared_ptr<Chunk>>;

class World
{
	public:
		World();
		virtual ~World();

		std::shared_ptr<Block> get_block(Position::BlockInWorld bwp) const;
		void set_block(Position::BlockInWorld bwp, std::shared_ptr<Block> block);

		void render_chunks();

		std::shared_ptr<Chunk> get_chunk(Position::ChunkInWorld cp) const;
		std::shared_ptr<Chunk> get_or_make_chunk(Position::ChunkInWorld cp);
		void set_chunk(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z, std::shared_ptr<Chunk> chunk);

		void gen_chunk(const Position::ChunkInWorld&);
		void gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max);

	private:
		map chunks;
		mutable uint64_t last_key;
		mutable std::shared_ptr<Chunk> last_chunk;
		std::minstd_rand random_engine;

		__attribute__((const))
		static uint64_t chunk_key(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z);
};