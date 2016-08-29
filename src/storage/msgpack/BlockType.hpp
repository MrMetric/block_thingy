#pragma once

#include "block/BlockType.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<BlockType>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, BlockType const t) const
	{
		o.pack(static_cast<block_type_id_t>(t));
		return o;
	}
};

template<>
struct convert<BlockType>
{
	msgpack::object const& operator()(msgpack::object const& o, BlockType& t) const
	{
		t = static_cast<BlockType>(o.as<block_type_id_t>());
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
