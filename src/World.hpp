#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <unordered_map>

#include "Player.hpp"
#include "position/ChunkInWorld.hpp"
#include "storage/WorldFile.hpp"

class Chunk;
class Block;
namespace Position
{
	struct BlockInWorld;
}

using world_map_keyhasher_t = std::function<uint_fast64_t(Position::ChunkInWorld)>;
using world_map_t = std::unordered_map<Position::ChunkInWorld, std::shared_ptr<Chunk>, world_map_keyhasher_t>;

class World
{
	friend class WorldFile;

	public:
		World(const std::string& file_path);
		World(const World&) = delete;

		const Block& get_block_const(const Position::BlockInWorld&) const;
		Block& get_block_mutable(const Position::BlockInWorld&);

		void set_block(const Position::BlockInWorld&, const Block&);

		std::shared_ptr<Chunk> get_chunk(const Position::ChunkInWorld&) const;
		std::shared_ptr<Chunk> get_or_make_chunk(const Position::ChunkInWorld&);
		void set_chunk(const Position::ChunkInWorld&, std::shared_ptr<Chunk> chunk);

		void gen_chunk(const Position::ChunkInWorld&);
		void gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max);

		void save();

	private:
		world_map_t chunks;
		mutable Position::ChunkInWorld last_key;
		mutable std::shared_ptr<Chunk> last_chunk;
		std::minstd_rand random_engine;

		std::vector<Player> players;
		WorldFile file;
};
