#pragma once

#include <limits>

#include "block/manager.hpp"
#include "storage/msgpack/block.hpp"

namespace block_thingy::block {

template<>
void manager::save(msgpack::packer<std::ofstream>& o) const
{
	o.pack_array(6);
	o.pack(generation);
	o.pack(free_indexes);
	o.pack(instance_count);
	o.pack(block_to_strid);
	o.pack(block_to_name);
	// pack_map takes uint32_t
	assert(components.size() <= std::numeric_limits<uint32_t>::max());
	o.pack_map(static_cast<uint32_t>(components.size()));
	for(const component::base* const c : components)
	{
		o.pack(c->get_id());
		c->save(o);
	}
}

template<>
void manager::load(const msgpack::object& o)
{
	if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
	if(o.via.array.size != 6) throw msgpack::type_error();
	const msgpack::object* a = o.via.array.ptr;
	if(a[4].type != msgpack::type::MAP) throw msgpack::type_error();

	generation = a[0].as<decltype(generation)>();
	free_indexes = a[1].as<decltype(free_indexes)>();
	instance_count = a[2].as<decltype(instance_count)>();
	block_to_strid = a[3].as<decltype(block_to_strid)>();
	strid_to_block.clear();
	for(const auto& [block, strid] : block_to_strid)
	{
		strid_to_block.emplace(strid, block);
	}
	block_to_name = a[4].as<decltype(block_to_name)>();

	const auto cm = a[5].as<std::map<std::string, msgpack::object>>();
	for(component::base* const c : components)
	{
		if(const auto i = cm.find(c->get_id());
			i != cm.cend())
		{
			c->load(i->second);
		}
	}
}

}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::block::manager;

template<>
struct pack<manager>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const manager& m) const
	{
		m.save(o);
		return o;
	}
};

template<>
struct convert<manager>
{
	const msgpack::object& operator()(const msgpack::object& o, manager& m) const
	{
		m.load(o);
		return o;
	}
};

} // namespace msgpack
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace adaptor
