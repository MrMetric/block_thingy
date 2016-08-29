#pragma once

#include "../../Player.hpp" // TODO

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

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

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
