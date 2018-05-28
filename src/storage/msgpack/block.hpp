#pragma once

#include <type_traits>

#include "block/block.hpp"
#include "block/enums/visibility_type.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::block_t;
using block_thingy::block::enums::visibility_type;

template<>
struct pack<block_t>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const block_t& block) const
	{
		o.pack((block.index << 8) | block.generation);
		return o;
	}
};

template<>
struct convert<block_t>
{
	const msgpack::object& operator()(const msgpack::object& o, block_t& block) const
	{
		if(o.type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();

		const uint32_t i = o.as<uint32_t>();
		block.index = i >> 8;
		block.generation = i & 0xFF;

		return o;
	}
};

template<>
struct pack<visibility_type>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const visibility_type& v) const
	{
		o.pack(static_cast<std::underlying_type_t<visibility_type>>(v));
		return o;
	}
};

template<>
struct convert<visibility_type>
{
	const msgpack::object& operator()(const msgpack::object& o, visibility_type& v) const
	{
		v = static_cast<visibility_type>(o.as<std::underlying_type_t<visibility_type>>());
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
