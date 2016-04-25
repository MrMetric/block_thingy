#include "WorldFile.hpp"

#include <string>
#include <utility>

#include <msgpack.hpp>

#include <Poco/File.h>

#include "Player.hpp"
#include "World.hpp"

WorldFile::WorldFile(const std::string& world_path, World& world)
	:
	world_path(world_path),
	player_path(world_path + "/players/"),
	chunk_path(world_path + "/chunks/"),
	world(world)
{
	Poco::File(player_path).createDirectories();
	Poco::File(chunk_path).createDirectories();
}

void WorldFile::save_players()
{
	for(const auto& p : world.players)
	{
		save_player(*p.second);
	}
}

void WorldFile::save_player(const Player& player)
{
	// TODO
}

std::unique_ptr<Player> WorldFile::load_player(const std::string& name)
{
	// TODO
	return nullptr;
}

void WorldFile::save_chunks()
{
	for(const auto& p : world.chunks)
	{
		save_chunk(*p.second);
	}
}

void WorldFile::save_chunk(const Chunk& chunk)
{
	// TODO
}

// should return unique_ptr, but shared_ptr is easier to deal with in World
std::shared_ptr<Chunk> WorldFile::load_chunk(const Position::ChunkInWorld& position)
{
	// TODO
	return nullptr;
}
