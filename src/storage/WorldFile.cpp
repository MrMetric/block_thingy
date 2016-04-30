#include "WorldFile.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <msgpack.hpp>
#include <Poco/File.h>

#include "msgpack_functions.hpp"

#include "Game.hpp"
#include "Player.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "console/Console.hpp"

#include "std_make_unique.hpp"

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
	std::ofstream stream(player_path + player.name, std::ios::binary);
	msgpack::pack(stream, player);
}

std::unique_ptr<Player> WorldFile::load_player(const std::string& name)
{
	std::string file_path = player_path + name;
	if(!Util::file_is_openable(file_path))
	{
		return nullptr;
	}

	(Game::instance != nullptr
		? Game::instance->console.logger
		: std::cout
	) << "loading player: " << file_path << "\n";

	std::string bytes = Util::read_file(file_path);
	msgpack::unpacked u;
	msgpack::unpack(u, bytes.c_str(), bytes.length());
	msgpack::object o = u.get();

	std::unique_ptr<Player> player = std::make_unique<Player>(name);
	o.convert(*player);

	return player;
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
