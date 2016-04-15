#pragma once

#include <memory>
#include <string>

class Player;
class World;
class Chunk;
namespace Position
{
	struct ChunkInWorld;
}

class WorldFile
{
	public:
		WorldFile(const std::string& world_path, World& world);

		WorldFile(WorldFile&&) = delete;
		WorldFile(const WorldFile&) = delete;
		void operator=(const WorldFile&) = delete;

		void save_players();
		void save_player(const Player&);
		std::unique_ptr<Player> load_player(const std::string& name);

		void save_chunks();
		void save_chunk(const Chunk&);
		std::shared_ptr<Chunk> load_chunk(const Position::ChunkInWorld&);

	private:
		std::string world_path;
		std::string player_path;
		std::string chunk_path;
		World& world;
};
