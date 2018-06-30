#pragma once

#include "world/world.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::world::world;

template<>
struct pack<world>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const world& world) const
	{
		world.save(o);
		return o;
	}
};

template<>
struct convert<world>
{
	const msgpack::object& operator()(const msgpack::object& o, world& world) const
	{
		world.load(o);
		return o;
	}
};

} // namespace msgpack
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace adaptor
