#include "WorldFile.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <msgpack.hpp>
#include <Poco/File.h>
#include <Poco/DeflatingStream.h>
#include <Poco/InflatingStream.h>
#include <Poco/StreamCopier.h>

#include "msgpack_functions.hpp"

#include "Game.hpp"
#include "Player.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "position/ChunkInWorld.hpp"

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

	LOGGER << "loading player: " << file_path << "\n";

	std::string bytes = Util::read_file(file_path);
	msgpack::unpacked u;
	msgpack::unpack(u, bytes.c_str(), bytes.length());
	msgpack::object o = u.get();

	auto player = std::make_unique<Player>(name);
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
	Position::ChunkInWorld position = chunk.get_position();
	std::string x = std::to_string(position.x);
	std::string y = std::to_string(position.y);
	std::string z = std::to_string(position.z);
	std::string file_path = chunk_path + x + "_" + y + "_" + z + ".gz";
	LOGGER << "saving " << file_path << "\n";

	std::ofstream stdstream(file_path, std::ios::binary);
	Poco::DeflatingOutputStream stream(stdstream, Poco::DeflatingStreamBuf::STREAM_GZIP);
	msgpack::pack(stream, chunk);
}

// should return unique_ptr, but shared_ptr is easier to deal with in World
std::shared_ptr<Chunk> WorldFile::load_chunk(const Position::ChunkInWorld& position)
{
	std::string x = std::to_string(position.x);
	std::string y = std::to_string(position.y);
	std::string z = std::to_string(position.z);
	std::string file_path = chunk_path + x + "_" + y + "_" + z + ".gz";
	if(!Util::file_is_openable(file_path))
	{
		return nullptr;
	}

	std::ifstream stdstream(file_path, std::ios::binary);
	Poco::InflatingInputStream stream(stdstream, Poco::InflatingStreamBuf::STREAM_GZIP);
	std::stringstream ss;
	Poco::StreamCopier::copyStream(stream, ss);
	std::string bytes = ss.str();

	msgpack::unpacked u;
	msgpack::unpack(u, bytes.c_str(), bytes.length());
	msgpack::object o = u.get();

	auto chunk = std::make_shared<Chunk>(position, world);
	o.convert(*chunk);

	return chunk;
}
