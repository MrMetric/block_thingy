#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <unordered_map>

class Chunk;
class Block;
namespace Position
{
	struct BlockInWorld;
	struct ChunkInWorld;
}

using map = std::unordered_map<uint64_t, std::shared_ptr<Chunk>>;

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
		mutable uint64_t last_key;
		mutable std::shared_ptr<Chunk> last_chunk;
		std::minstd_rand random_engine;

		static uint64_t chunk_key(const Position::ChunkInWorld&);
};
