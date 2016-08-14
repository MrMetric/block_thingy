#pragma once

#include <functional>
#include <memory>
#include <set>
#include <stdint.h>
#include <string>
#include <unordered_map>

#include "chunk/mesh/ChunkMesher.hpp"
#include "position/ChunkInWorld.hpp"
#include "storage/WorldFile.hpp"

class Player;
class Chunk;
namespace Block
{
	class Block;
}
namespace Graphics
{
	struct Color;
}
namespace Position
{
	struct BlockInWorld;
}

using world_map_keyhasher_t = std::function<uint64_t(Position::ChunkInWorld)>;
using world_map_t = std::unordered_map<Position::ChunkInWorld, std::shared_ptr<Chunk>, world_map_keyhasher_t>;

class World
{
	friend class WorldFile;

	public:
		World(const std::string& file_path);

		World(World&&) = delete;
		World(const World&) = delete;
		void operator=(const World&) = delete;

		Block::Block get_block(const Position::BlockInWorld&) const;

		void set_block(const Position::BlockInWorld&, const Block::Block&);

		Graphics::Color get_light(const Position::BlockInWorld&) const;
		void set_light(const Position::BlockInWorld&, const Graphics::Color&);
		void add_light(const Position::BlockInWorld&, const Graphics::Color&);

		std::shared_ptr<Chunk> get_chunk(const Position::ChunkInWorld&) const;
		std::shared_ptr<Chunk> get_or_make_chunk(const Position::ChunkInWorld&);
		void set_chunk(const Position::ChunkInWorld&, std::shared_ptr<Chunk> chunk);

		void gen_chunk(const Position::ChunkInWorld&);
		void gen_at(const Position::BlockInWorld& min, const Position::BlockInWorld& max);

		void step(double delta_time);

		std::shared_ptr<Player> add_player(const std::string& name);
		std::shared_ptr<Player> get_player(const std::string& name);

		void save();

		std::unique_ptr<ChunkMesher> mesher;

	private:
		world_map_t chunks;
		std::set<Position::ChunkInWorld> chunks_to_save;
		mutable Position::ChunkInWorld last_key;
		mutable std::shared_ptr<Chunk> last_chunk;

		std::unordered_map<std::string, std::shared_ptr<Player>> players;
		WorldFile file;
};
