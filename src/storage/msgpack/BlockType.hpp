#pragma once

#include "block/Enum/Type.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<Block::Enum::TypeExternal>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const Block::Enum::TypeExternal t) const
	{
		o.pack(static_cast<Block::Enum::Type_t>(t));
		return o;
	}
};

template<>
struct convert<Block::Enum::TypeExternal>
{
	const msgpack::object& operator()(const msgpack::object& o, Block::Enum::TypeExternal& t) const
	{
		t = static_cast<Block::Enum::TypeExternal>(o.as<Block::Enum::Type_t>());
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
