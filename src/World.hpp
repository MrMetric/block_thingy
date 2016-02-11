#pragma once

#include <cstdint>
#include <memory>
#include <random>

#include <unordered_map>

#include <glad/glad.h>

#include "Coords.hpp"

class Chunk;
class Block;

using map = std::unordered_map<uint64_t, std::shared_ptr<Chunk>>;

class World
{
	public:
		World(GLint vs_cube_pos_mod);
		virtual ~World();

		Block get_block(const Position::BlockInWorld) const;
		void set_block(const Position::BlockInWorld, Block);

		std::shared_ptr<Chunk> get_chunk(const Position::ChunkInWorld) const;
		std::shared_ptr<Chunk> get_or_make_chunk(const Position::ChunkInWorld);
		void set_chunk(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z, std::shared_ptr<Chunk> chunk);

		void gen_chunk(const Position::ChunkInWorld&);
		void gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max);

		GLint vs_cube_pos_mod;

	private:
		map chunks;
		mutable uint64_t last_key;
		mutable std::shared_ptr<Chunk> last_chunk;
		std::minstd_rand random_engine;

		static uint64_t chunk_key(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z);
};
