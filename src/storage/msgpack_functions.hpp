#pragma once

#include <string>
#include <unordered_map>

#include <msgpack.hpp>

#include <glm/vec3.hpp>

#include "Block.hpp"
#include "Player.hpp"
#include "chunk/Chunk.hpp"

template <typename T>
bool find_in_map(const std::unordered_map<std::string, msgpack::object>& map, const std::string& key, T& v)
{
	auto i = map.find(key);
	if(i != map.end())
	{
		v = i->second.as<T>();
		return true;
	}
	return false;
}

template <typename T>
void find_in_map_or_throw(const std::unordered_map<std::string, msgpack::object>& map, const std::string& key, T& v)
{
	if(!find_in_map(map, key, v))
	{
		throw msgpack::type_error();
	}
}

template <typename T>
void unpack_bytes(const std::string& bytes, T& v)
{
	msgpack::unpacked u;
	msgpack::unpack(u, bytes.c_str(), bytes.length());
	msgpack::object o = u.get();
	o.convert(v);
}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<glm::dvec3>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, glm::dvec3 const& v) const
	{
		o.pack_array(3);
		o.pack(v.x);
		o.pack(v.y);
		o.pack(v.z);
		return o;
	}
};

template<>
struct convert<glm::dvec3>
{
	msgpack::object const& operator()(msgpack::object const& o, glm::dvec3& v) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size != 3) throw msgpack::type_error();

		v.x = o.via.array.ptr[0].as<double>();
		v.y = o.via.array.ptr[1].as<double>();
		v.z = o.via.array.ptr[2].as<double>();

		return o;
	}
};

template<>
struct pack<Player>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Player const& player) const
	{
		bool noclip = player.get_noclip();
		o.pack_map(noclip ? 4 : 3);
		o.pack("position"); o.pack(player.position);
		o.pack("rotation"); o.pack(player.rotation);
		o.pack("velocity"); o.pack(player.velocity);
		if(noclip)
		{
			o.pack("noclip"); o.pack(player.get_noclip());
		}
		return o;
	}
};

template<>
struct convert<Player>
{
	msgpack::object const& operator()(msgpack::object const& o, Player& player) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 3) throw msgpack::type_error();

		auto map = o.as<std::unordered_map<std::string, msgpack::object>>();

		find_in_map_or_throw(map, "position", player.position);
		find_in_map_or_throw(map, "rotation", player.rotation);
		find_in_map_or_throw(map, "velocity", player.velocity);
		bool noclip = false;
		find_in_map(map, "noclip", noclip);
		player.set_noclip(noclip);

		return o;
	}
};

template<>
struct pack<Chunk>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Chunk const& chunk) const
	{
		o.pack_array(2);
		bool is_solid = chunk.blocks == nullptr;
		o.pack(is_solid);
		if(is_solid)
		{
			o.pack(chunk.solid_block);
		}
		else
		{
			o.pack(chunk.blocks);
		}
		return o;
	}
};

template<>
struct convert<Chunk>
{
	msgpack::object const& operator()(msgpack::object const& o, Chunk& chunk) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size != 2) throw msgpack::type_error();

		bool is_solid = o.via.array.ptr[0].as<bool>();
		if(is_solid)
		{
			chunk.blocks = nullptr; // NOTE: should be null already
			chunk.solid_block = o.via.array.ptr[1].as<Block>();
		}
		else
		{
			// let us hope this copy is optimized out
			chunk.blocks = std::make_unique<chunk_block_array_t>(o.via.array.ptr[1].as<chunk_block_array_t>());
		}

		return o;
	}
};

template<>
struct pack<Block>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Block const& block) const
	{
		o.pack_map(1);

		o.pack("t"); o.pack(block.type_id());

		return o;
	}
};

template<>
struct convert<Block>
{
	msgpack::object const& operator()(msgpack::object const& o, Block& block) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 1) throw msgpack::type_error();

		auto map = o.as<std::unordered_map<std::string, msgpack::object>>();

		block_type_id_t type_id;
		find_in_map_or_throw(map, "t", type_id);
		const BlockType block_type = static_cast<BlockType>(type_id);
		block = Block(block_type);

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
