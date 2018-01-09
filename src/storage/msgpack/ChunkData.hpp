#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>

#include "chunk/ChunkData.hpp"

namespace block_thingy {

using T = std::shared_ptr<block::base>;

template<>
template<>
void ChunkData<T>::save(msgpack::packer<zstr::ostream>& o) const
{
	o.pack_array(2);

	std::vector<T> block_vec;
	std::unordered_map<T::element_type*, uint32_t> block_map;
	uint32_t block_i = 0;
	for(auto& block : this->blocks)
	{
		auto i = block_map.find(block.get());
		if(i == block_map.cend())
		{
			block_vec.emplace_back(block);
			block_map.emplace(block.get(), block_i++);
		}
	}

	o.pack(block_vec);

	assert(this->blocks.size() <= std::numeric_limits<uint32_t>::max());
	o.pack_array(static_cast<uint32_t>(this->blocks.size()));
	for(auto& block : this->blocks)
	{
		o.pack(block_map[block.get()]);
	}
}

template<>
template<>
void ChunkData<T>::load(const msgpack::object& o)
{
	if(o.type != msgpack::type::ARRAY)
	{
		throw msgpack::type_error();
	}
	if(o.via.array.size != 2)
	{
		throw msgpack::type_error();
	}
	const auto v = o.as<std::array<msgpack::object, 2>>();

	const auto block_vec = v[0].as<std::vector<T>>();

	if(v[1].type != msgpack::type::ARRAY)
	{
		throw msgpack::type_error();
	}
	// msgpack errors on > instead of !=
	if(v[1].via.array.size != CHUNK_BLOCK_COUNT)
	{
		throw msgpack::type_error();
	}
	const auto block_map = v[1].as<std::array<uint32_t, CHUNK_BLOCK_COUNT>>();
	for(std::size_t i = 0; i < CHUNK_BLOCK_COUNT; ++i)
	{
		this->blocks[i] = block_vec[block_map[i]];
	}
}

}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::ChunkData;

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
