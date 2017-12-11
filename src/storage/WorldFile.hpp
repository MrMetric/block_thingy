#pragma once

#include <memory>
#include <string>

#include "fwd/Player.hpp"
#include "fwd/World.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "fwd/position/ChunkInWorld.hpp"
#include "util/filesystem.hpp"

namespace Storage {

class WorldFile
{
public:
	WorldFile(const fs::path& world_dir, World& world);

	WorldFile(WorldFile&&) = delete;
	WorldFile(const WorldFile&) = delete;
	WorldFile& operator=(WorldFile&&) = delete;
	WorldFile& operator=(const WorldFile&) = delete;

	void save_world();

	/**
	 * Save all players that are currently in the world
	 *
	 * @note Does not skip players that are already saved
	 */
	void save_players();

	/**
	 * Save a player
	 *
	 * @note The player does not need to be in the world
	 * @warning Overwrites any player with the same name
	 * @todo Use UUIDs instead of names
	 */
	void save_player(const Player&);

	/**
	 * Load the player that has the specified name. If the player does not exist, a new one is created.
	 */
	std::unique_ptr<Player> load_player
	(
		const std::string& name
	);

	/**
	 * Save a chunk
	 *
	 * @note The chunk does not need to be in the world
	 * @warning Overwrites any chunk at the same position
	 */
	void save_chunk(const Chunk&);

	/**
	 * Load the chunk that is at specified position. If the chunk does not exist, `nullptr` is returned.
	 */
	std::unique_ptr<Chunk> load_chunk(const Position::ChunkInWorld&);

	bool has_chunk(const Position::ChunkInWorld&);

private:
	fs::path world_path;
	fs::path player_dir;
	fs::path chunk_dir;
	World& world;

	fs::path chunk_path(const Position::ChunkInWorld&);
};

}
