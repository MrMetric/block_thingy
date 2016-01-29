#pragma once

#include <boost/unordered_map.hpp>

#include "Chunk.hpp"

using map = boost::unordered_map<uint64_t, Chunk*>;

class World
{
	public:
		World();
		virtual ~World();

		Block* get_block(Position::BlockInWorld bwp);
		void set_block(Position::BlockInWorld bwp, Block* block, bool delete_old_block = true);

		void render_chunks();

		Chunk* get_chunk(Position::ChunkInWorld cp, bool create_if_null = false);
		void set_chunk(int32_t x, int32_t y, int32_t z, Chunk* chunk);

		void gen_chunk(const Position::ChunkInWorld&);
		void gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max);

	private:
		map chunks;
		uint64_t last_key;
		Chunk* last_chunk;

		__attribute__((const))
		static uint64_t chunk_key(int32_t x, int32_t y, int32_t z);
};