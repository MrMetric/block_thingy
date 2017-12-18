#pragma once

#include "block/Enum/Type.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::block::enums::type_t;
using block_thingy::block::enums::type_external;

template<>
struct pack<type_external>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const type_external t) const
	{
		o.pack(static_cast<type_t>(t));
		return o;
	}
};

template<>
struct convert<type_external>
{
	const msgpack::object& operator()(const msgpack::object& o, type_external& t) const
	{
		t = static_cast<type_external>(o.as<type_t>());
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
