#pragma once

#include <vector>

#include "game.hpp"
#include "storage/msgpack/block_manager.hpp"
#include "world/world.hpp"

namespace block_thingy::world {

//template<typename Stream>
//void world::save(msgpack::packer<Stream>& o) const
// that does not work :[
template<>
void world::save(msgpack::packer<std::ofstream>& o) const
{
	o.pack_array(4);
	o.pack(get_name());
	o.pack(get_seed());
	o.pack(get_ticks());
	o.pack(block_manager);
}

template<>
void world::load(const msgpack::object& o)
{
	if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
	if(o.via.array.size != 4) throw msgpack::type_error();
	const auto& a = o.via.array.ptr;

	set_name(a[0].as<std::string>());
	set_seed(a[1].as<double>());
	set_ticks(a[2].as<uint64_t>());
	block_manager.load(a[3]);
}

}

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
