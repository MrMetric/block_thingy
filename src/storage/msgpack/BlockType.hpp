#pragma once

#include "block/BlockType.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<BlockType>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const BlockType t) const
	{
		o.pack(static_cast<block_type_id_t>(t));
		return o;
	}
};

template<>
struct convert<BlockType>
{
	const msgpack::object& operator()(const msgpack::object& o, BlockType& t) const
	{
		t = static_cast<BlockType>(o.as<block_type_id_t>());
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
