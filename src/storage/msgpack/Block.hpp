#pragma once

#include <memory>

#include "Game.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/BlockType.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<Block::Base>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const Block::Base& block) const
	{
		Storage::OutputInterface i;
		block.save(i);
		i.flush(o);

		return o;
	}
};

template<>
struct convert<std::unique_ptr<Block::Base>>
{
	const msgpack::object& operator()(const msgpack::object& o, std::unique_ptr<Block::Base>& block) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 1) throw msgpack::type_error();

		const auto map = o.as<std::map<std::string, msgpack::object>>();

		BlockTypeExternal t;
		find_in_map_or_throw(map, "", t);
		block = Game::instance->block_registry.make(t);
		Storage::InputInterface i(map);
		block->load(i);

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
