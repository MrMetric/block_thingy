#pragma once

#include "chunk/ChunkData.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<typename T>
struct pack<ChunkData<T>>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const ChunkData<T>& chunk_data) const
	{
		o.pack(chunk_data.blocks);
		return o;
	}
};

template<typename T>
struct convert<ChunkData<T>>
{
	const msgpack::object& operator()(const msgpack::object& o, ChunkData<T>& chunk_data) const
	{
		chunk_data.blocks = o.as<typename ChunkData<T>::chunk_data_t>();
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
