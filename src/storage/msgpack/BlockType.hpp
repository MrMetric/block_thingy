#pragma once

#include "block/Enum/Type.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::block::enums::Type_t;
using block_thingy::block::enums::TypeExternal;

template<>
struct pack<TypeExternal>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const TypeExternal t) const
	{
		o.pack(static_cast<Type_t>(t));
		return o;
	}
};

template<>
struct convert<TypeExternal>
{
	const msgpack::object& operator()(const msgpack::object& o, TypeExternal& t) const
	{
		t = static_cast<TypeExternal>(o.as<Type_t>());
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
