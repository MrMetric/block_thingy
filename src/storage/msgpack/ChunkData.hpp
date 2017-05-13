#pragma once

#include "chunk/ChunkData.hpp"

using T = std::unique_ptr<Block::Base>;

template<>
template<>
void ChunkData<T>::save(msgpack::packer<zstr::ostream>& o) const
{
	o.pack_array(2);
	const bool is_solid = (blocks == nullptr);
	o.pack(is_solid);
	if(is_solid)
	{
		o.pack(solid_block);
	}
	else
	{
		o.pack(*blocks);
	}
}

template<>
template<>
void ChunkData<T>::load(const msgpack::object& o)
{
	const auto v = o.as<std::vector<msgpack::object>>();
	decltype(v)::size_type i = 0;

	const bool is_solid = v.at(i++).as<bool>();
	if(is_solid)
	{
		fill(v.at(i++).as<T>());
	}
	else
	{
		const msgpack::object& o = v.at(i++);
		// these checks are separate to distinguish them when debugging
		if(o.type != msgpack::type::ARRAY)
		{
			throw msgpack::type_error();
		}
		// msgpack errors on > instead of !=
		if(o.via.array.size != CHUNK_BLOCK_COUNT)
		{
			throw msgpack::type_error();
		}
		blocks = o.as<std::unique_ptr<typename ChunkData<T>::chunk_data_t>>();
	}
}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<typename T>
struct pack<ChunkData<T>>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const ChunkData<T>& chunk_data) const
	{
		chunk_data.save(o);
		return o;
	}
};

template<typename T>
struct convert<ChunkData<T>>
{
	const msgpack::object& operator()(const msgpack::object& o, ChunkData<T>& chunk_data) const
	{
		chunk_data.load(o);
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
