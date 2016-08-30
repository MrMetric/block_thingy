#pragma once

#include <memory>

#include "Game.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "storage/Interface.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<Block::Base>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Block::Base const& block) const
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
	msgpack::object const& operator()(msgpack::object const& o, std::unique_ptr<Block::Base>& block) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 1) throw msgpack::type_error();

		auto map = o.as<std::unordered_map<std::string, msgpack::object>>();

		BlockType t;
		find_in_map_or_throw(map, "t", t);
		block = Game::instance->block_registry.make(t);
		Storage::InputInterface i(map);
		block->load(i);

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
