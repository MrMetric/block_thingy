#pragma once

#include "graphics/color.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::graphics::color;

template<>
struct pack<color>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const color& color) const
	{
		o.pack_array(3);
		o.pack(color.r);
		o.pack(color.g);
		o.pack(color.b);
		return o;
	}
};

template<>
struct convert<color>
{
	const msgpack::object& operator()(const msgpack::object& o, color& color) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size != 3) throw msgpack::type_error();
		const auto& a = o.via.array.ptr;

		color.r = a[0].as<decltype(color.r)>();
		color.g = a[1].as<decltype(color.g)>();
		color.b = a[2].as<decltype(color.b)>();

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
