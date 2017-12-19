#pragma once

#include <memory>

#include "game.hpp"
#include "block/base.hpp"
#include "block/BlockRegistry.hpp"
#include "block/enums/type.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/block_type.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::block::base;
using block_thingy::storage::find_in_map_or_throw;
using block_thingy::storage::InputInterface;
using block_thingy::storage::OutputInterface;

template<>
struct pack<base>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const base& block) const
	{
		OutputInterface i;
		block.save(i);
		i.flush(o);

		return o;
	}
};

template<>
struct convert<std::shared_ptr<base>>
{
	const msgpack::object& operator()(const msgpack::object& o, std::shared_ptr<base>& block) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 1) throw msgpack::type_error();

		const auto map = o.as<std::map<std::string, msgpack::object>>();

		block_thingy::block::enums::type_external t;
		find_in_map_or_throw(map, "", t);
		block_thingy::block::BlockRegistry& block_registry = block_thingy::game::instance->block_registry;
		block = block_registry.make(block_registry.get_default(t)); // TODO
		InputInterface i(map);
		block->load(i);

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
