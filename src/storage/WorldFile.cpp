#include "WorldFile.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <easylogging++/easylogging++.hpp>
#include <msgpack.hpp>
#include <zstr/zstr.hpp>

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
#include "util/copy_stream.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::to_string;
using std::shared_ptr;
using std::unique_ptr;

namespace Storage {

WorldFile::WorldFile(const string& world_dir, World& world)
:
	world_path(world_dir + "/world"),
	player_dir(world_dir + "/players/"),
	chunk_dir(world_dir + "/chunks/"),
	world(world)
{
	Util::create_directories(player_dir);
	Util::create_directories(chunk_dir);

	if(!Util::file_is_openable(world_path))
	{
		return;
	}
	LOG(INFO) << "loading world: " << world_path;
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
	std::ofstream stream(player_dir + player.name, std::ofstream::binary);
	msgpack::pack(stream, player);
}

unique_ptr<Player> WorldFile::load_player
(
	const string& name
)
{
	auto player = std::make_unique<Player>(*Game::instance, name);

	string file_path = player_dir + name;
	if(!Util::file_is_openable(file_path))
	{
		return player;
	}

	LOG(INFO) << "loading player: " << file_path;

	string bytes = Util::read_file(file_path);
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
	string file_path = chunk_dir + x + "_" + y + "_" + z + ".gz";
	LOG(INFO) << "saving " << file_path;

	std::ofstream stdstream(file_path, std::ofstream::binary);
	zstr::ostream stream(stdstream);
	msgpack::pack(stream, chunk);
}

// should return unique_ptr, but shared_ptr is easier to deal with in World
shared_ptr<Chunk> WorldFile::load_chunk(const Position::ChunkInWorld& position)
{
	string x = to_string(position.x);
	string y = to_string(position.y);
	string z = to_string(position.z);
	string file_path = chunk_dir + x + "_" + y + "_" + z + ".gz";
	if(!Util::file_is_openable(file_path))
	{
		return nullptr;
	}

	std::ifstream stdstream(file_path, std::ifstream::binary);
	zstr::istream stream(stdstream);
	string bytes = Util::read_stream(stream);
	auto chunk = std::make_shared<Chunk>(position, world);
	try
	{
		unpack_bytes(bytes, *chunk);
	}
	catch(const msgpack::v1::insufficient_bytes& e)
	{
		// TODO: load truncated chunks
		LOG(ERROR) << "error loading " << file_path << ": " << e.what();
		return nullptr;
	}
	catch(const msgpack::type_error& e)
	{
		//throw std::runtime_error("error loading " + file_path + ": " + e.what());
		LOG(ERROR) << "error loading " << file_path << ": " << e.what();
		// TODO: rename the bad file so the user can attempt to recover it (because the new chunk will overwrite it)
		return nullptr;
	}
	//catch(const std::exception& e)

	return chunk;
}

} // namespace Storage
