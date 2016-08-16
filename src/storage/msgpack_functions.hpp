#pragma once

#include <string>
#include <unordered_map>

#include <msgpack.hpp>

#include <glm/vec3.hpp>

#include "Player.hpp"
#include "block/Block.hpp"
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

template<typename T>
struct pack<glm::tvec3<T>>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, glm::tvec3<T> const& v) const
	{
		o.pack_array(3);
		o.pack(v.x);
		o.pack(v.y);
		o.pack(v.z);
		return o;
	}
};

template<typename T>
struct convert<glm::tvec3<T>>
{
	msgpack::object const& operator()(msgpack::object const& o, glm::tvec3<T>& v) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size != 3) throw msgpack::type_error();

		v.x = o.via.array.ptr[0].as<T>();
		v.y = o.via.array.ptr[1].as<T>();
		v.z = o.via.array.ptr[2].as<T>();

		return o;
	}
};

template<>
struct pack<Player>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Player const& player) const
	{
		const bool noclip = player.get_noclip();
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
		const bool has_meshes = false;
		uint32_t array_size = 3;
		if(has_meshes) array_size += 1;

		o.pack_array(array_size);
		const bool is_solid = chunk.blocks == nullptr;
		o.pack(is_solid);
		if(is_solid)
		{
			o.pack(chunk.solid_block);
		}
		else
		{
			o.pack(chunk.blocks);
		}

		o.pack(has_meshes);
		if(has_meshes)
		{
			o.pack(chunk.get_meshes());
		}

		return o;
	}
};

template <typename T> class butts;

template<>
struct convert<Chunk>
{
	msgpack::object const& operator()(msgpack::object const& o, Chunk& chunk) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size < 3) throw msgpack::type_error();

		auto array = o.via.array.ptr;
		uint_fast8_t i = 0;

		const bool is_solid = array[i++].as<bool>();
		if(is_solid)
		{
			chunk.set_blocks(array[i++].as<Block::Block>());
		}
		else
		{
			// let us hope this copy is optimized out
			chunk.set_blocks(std::make_unique<chunk_block_array_t>(array[i++].as<chunk_block_array_t>()));
		}

		const bool has_meshes = array[i++].as<bool>();
		if(has_meshes)
		{
			chunk.set_meshes(array[i++].as<meshmap_t>());
		}

		return o;
	}
};

template<>
struct pack<Block::Block>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Block::Block const& block) const
	{
		o.pack_map(1);

		o.pack("t"); o.pack(block.type_id());

		return o;
	}
};

template<>
struct convert<Block::Block>
{
	msgpack::object const& operator()(msgpack::object const& o, Block::Block& block) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 1) throw msgpack::type_error();

		auto map = o.as<std::unordered_map<std::string, msgpack::object>>();

		block_type_id_t type_id;
		find_in_map_or_throw(map, "t", type_id);
		const BlockType block_type = static_cast<BlockType>(type_id);
		block = Block::Block(block_type);

		return o;
	}
};

template<>
struct pack<BlockType>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, BlockType const t) const
	{
		o.pack(static_cast<block_type_id_t>(t));
		return o;
	}
};

template<>
struct convert<BlockType>
{
	msgpack::object const& operator()(msgpack::object const& o, BlockType& t) const
	{
		t = static_cast<BlockType>(o.as<block_type_id_t>());
		return o;
	}
};

template<>
struct pack<Graphics::Color>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Graphics::Color const& color) const
	{
		o.pack_array(3);
		o.pack(color.r);
		o.pack(color.g);
		o.pack(color.b);
		return o;
	}
};

template<>
struct convert<Graphics::Color>
{
	msgpack::object const& operator()(msgpack::object const& o, Graphics::Color& color) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size != 3) throw msgpack::type_error();

		color.r = o.via.array.ptr[0].as<decltype(color.r)>();
		color.g = o.via.array.ptr[1].as<decltype(color.g)>();
		color.b = o.via.array.ptr[2].as<decltype(color.b)>();

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
