#pragma once

#include <vector>

#include "World.hpp"

//template<typename Stream>
//void World::save(msgpack::packer<Stream>& o) const
// that does not work :[
template<>
void World::save(msgpack::packer<std::ofstream>& o) const
{
	o.pack_array(1);
	o.pack(ticks);
}

template<>
void World::load(const msgpack::object& o)
{
	const auto v = o.as<std::vector<msgpack::object>>();
	ticks = v.at(0).as<decltype(ticks)>();
}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<World>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const World& world) const
	{
		world.save(o);
		return o;
	}
};

template<>
struct convert<World>
{
	const msgpack::object& operator()(const msgpack::object& o, World& world) const
	{
		world.load(o);
		return o;
	}
};

} // namespace msgpack
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace adaptor
