#pragma once

#include <cstdint>

#include <boost/unordered/unordered_map.hpp>

#include "Coords.hpp"

class Chunk;
class Block;

using map = boost::unordered_map<uint64_t, Chunk*>;

class World
{
	public:
		World();
		virtual ~World();

		Block* get_block(Position::BlockInWorld bwp) const;
		void set_block(Position::BlockInWorld bwp, Block* block, bool delete_old_block = true);

		void render_chunks();

		Chunk* get_chunk(Position::ChunkInWorld cp) const;
		Chunk* get_or_make_chunk(Position::ChunkInWorld cp);
		void set_chunk(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z, Chunk* chunk);

		void gen_chunk(const Position::ChunkInWorld&);
		void gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max);

	private:
		map chunks;
		mutable uint64_t last_key;
		mutable Chunk* last_chunk;

		__attribute__((const))
		static uint64_t chunk_key(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z);
};