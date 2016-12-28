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

#include "Game.hpp"
#include "Player.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "position/ChunkInWorld.hpp"
#include "storage/msgpack_util.hpp"
#include "storage/msgpack/Player.hpp"
#include "storage/msgpack/World.hpp"
#include "storage/msgpack/Chunk.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::to_string;
using std::shared_ptr;
using std::unique_ptr;

namespace Storage {

WorldFile::WorldFile(const string& world_dir, World& world)
:
	world_path(world_dir + "/world"),
	player_path(world_dir + "/players/"),
	chunk_path(world_dir + "/chunks/"),
	world(world)
{
	Poco::File(player_path).createDirectories();
	Poco::File(chunk_path).createDirectories();

	if(!Util::file_is_openable(world_path))
	{
		return;
	}
	LOGGER << "loading world: " << world_path << "\n";
	string bytes = Util::read_file(world_path);
	try
	{
		unpack_bytes(bytes, world);
	}
	catch(const msgpack::type_error& e)
	{
		throw std::runtime_error("error loading " + world_path + ": " + e.what());
	}
}

void WorldFile::save_world()
{
	std::ofstream stream(world_path, std::ofstream::binary);
	msgpack::pack(stream, world);
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
	std::ofstream stream(player_path + player.name, std::ofstream::binary);
	msgpack::pack(stream, player);
}

unique_ptr<Player> WorldFile::load_player
(
	Game& game,
	const string& name
)
{
	string file_path = player_path + name;
	if(!Util::file_is_openable(file_path))
	{
		return nullptr;
	}

	LOGGER << "loading player: " << file_path << "\n";

	string bytes = Util::read_file(file_path);
	auto player = std::make_unique<Player>(game, name);
	try
	{
		unpack_bytes(bytes, *player);
	}
	catch(const msgpack::type_error& e)
	{
		throw std::runtime_error("error loading " + file_path + ": " + e.what());
	}

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
	string x = to_string(position.x);
	string y = to_string(position.y);
	string z = to_string(position.z);
	string file_path = chunk_path + x + "_" + y + "_" + z + ".gz";
	LOGGER << "saving " << file_path << "\n";

	std::ofstream stdstream(file_path, std::ofstream::binary);
	Poco::DeflatingOutputStream stream(stdstream, Poco::DeflatingStreamBuf::STREAM_GZIP);
	msgpack::pack(stream, chunk);
}

// should return unique_ptr, but shared_ptr is easier to deal with in World
shared_ptr<Chunk> WorldFile::load_chunk(const Position::ChunkInWorld& position)
{
	string x = to_string(position.x);
	string y = to_string(position.y);
	string z = to_string(position.z);
	string file_path = chunk_path + x + "_" + y + "_" + z + ".gz";
	if(!Util::file_is_openable(file_path))
	{
		return nullptr;
	}

	std::ifstream stdstream(file_path, std::ifstream::binary);
	Poco::InflatingInputStream stream(stdstream, Poco::InflatingStreamBuf::STREAM_GZIP);
	std::stringstream ss;
	Poco::StreamCopier::copyStream(stream, ss);
	string bytes = ss.str();
	auto chunk = std::make_shared<Chunk>(position, world);
	try
	{
		unpack_bytes(bytes, *chunk);
	}
	catch(const msgpack::type_error& e)
	{
		//throw std::runtime_error("error loading " + file_path + ": " + e.what());
		world.game.console.error_logger << "error loading " << file_path << ": " << e.what() << "\n";
		// TODO: rename the bad file so the user can attempt to recover it (because the new chunk will overwrite it)
		return nullptr;
	}

	return chunk;
}

} // namespace Storage
