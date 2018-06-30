#pragma once

#include "position/block_in_chunk.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

#define INSTANTIATE(T) \
template<> \
struct pack<T> \
{ \
	template<typename Stream> \
	packer<Stream>& operator()(packer<Stream>& o, const T& pos) const \
	{ \
		o.pack_array(3); \
		o.pack(pos.x); \
		o.pack(pos.y); \
		o.pack(pos.z); \
		return o; \
	} \
}; \
\
template<> \
struct convert<T> \
{ \
	const msgpack::object& operator()(const msgpack::object& o, T& pos) const \
	{ \
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error(); \
		if(o.via.array.size != 3) throw msgpack::type_error(); \
\
		pos.x = o.via.array.ptr[0].as<decltype(pos.x)>(); \
		pos.y = o.via.array.ptr[1].as<decltype(pos.y)>(); \
		pos.z = o.via.array.ptr[2].as<decltype(pos.z)>(); \
\
		return o; \
	} \
}; \

INSTANTIATE(block_thingy::position::block_in_chunk)
INSTANTIATE(block_thingy::position::block_in_world)
INSTANTIATE(block_thingy::position::chunk_in_world)

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
