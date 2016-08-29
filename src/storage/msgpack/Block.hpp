#pragma once

#include <memory>

#include "Game.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<Block::Block>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Block::Block const& block) const
	{
		o.pack_map(1);

		BlockType t = block.type() != BlockType::none ? block.type() : BlockType::air;
		o.pack("t"); o.pack(t);

		return o;
	}
};

template<>
struct convert<std::unique_ptr<Block::Block>>
{
	msgpack::object const& operator()(msgpack::object const& o, std::unique_ptr<Block::Block>& block) const
	{
		if(o.type != msgpack::type::MAP) throw msgpack::type_error();
		if(o.via.map.size < 1) throw msgpack::type_error();

		auto map = o.as<std::unordered_map<std::string, msgpack::object>>();

		BlockType t;
		find_in_map_or_throw(map, "t", t);
		block = Game::instance->block_registry.make(t);

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
