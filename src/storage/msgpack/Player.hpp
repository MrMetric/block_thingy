#pragma once

#include "../../Player.hpp" // TODO
#include "storage/msgpack/glm_vec3.hpp"
#include "storage/msgpack/Property.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::Player;
using block_thingy::storage::find_in_map;
using block_thingy::storage::find_in_map_or_throw;

template<>
struct pack<Player>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const Player& player) const
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
	const msgpack::object& operator()(const msgpack::object& o, Player& player) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 3) throw msgpack::type_error();

		auto map = o.as<std::map<std::string, msgpack::object>>();

		find_in_map_or_throw(map, "position", player.position);
		find_in_map_or_throw(map, "rotation", player.rotation);
		find_in_map_or_throw(map, "velocity", player.velocity);
		bool noclip = false;
		find_in_map(map, "noclip", noclip);
		player.set_noclip(noclip);

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
