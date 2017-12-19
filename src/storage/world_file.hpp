#pragma once

#include <memory>
#include <string>

#include "fwd/Player.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "fwd/position/chunk_in_world.hpp"
#include "util/filesystem.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy::storage {

class world_file
{
public:
	world_file(const fs::path& world_dir, world::world& world);

	world_file(world_file&&) = delete;
	world_file(const world_file&) = delete;
	world_file& operator=(world_file&&) = delete;
	world_file& operator=(const world_file&) = delete;

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
	std::unique_ptr<Chunk> load_chunk(const position::chunk_in_world&);

	bool has_chunk(const position::chunk_in_world&);

private:
	fs::path world_path;
	fs::path player_dir;
	fs::path chunk_dir;
	world::world& world;

	fs::path chunk_path(const position::chunk_in_world&);
};

}
