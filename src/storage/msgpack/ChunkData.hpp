#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>

#include "chunk/ChunkData.hpp"

namespace block_thingy {

#define INSTANTIATE(T) \
template<> \
template<> \
void chunk_data<T>::save(msgpack::packer<zstr::ostream>& o) const \
{ \
	o.pack(blocks); \
} \
\
template<> \
template<> \
void chunk_data<T>::load(const msgpack::object& o) \
{ \
	if(o.type != msgpack::type::ARRAY) throw msgpack::type_error(); \
	if(o.via.array.size != blocks.size()) throw msgpack::type_error(); \
	blocks = o.as<decltype(blocks)>(); \
} \

INSTANTIATE(block_t)
INSTANTIATE(graphics::color)

#undef INSTANTIATE

}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::chunk_data;

template<typename T>
struct pack<chunk_data<T>>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const chunk_data<T>& data) const
	{
		data.save(o);
		return o;
	}
};

template<typename T>
struct convert<chunk_data<T>>
{
	const msgpack::object& operator()(const msgpack::object& o, chunk_data<T>& data) const
	{
		data.load(o);
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
