#pragma once

#include <vector>

#include "chunk/Chunk.hpp"
#include "storage/msgpack/Block.hpp"
#include "storage/msgpack/ChunkData.hpp"

#include "std_make_unique.hpp"

template<>
void Chunk::save(msgpack::packer<zstr::ostream>& o) const
{
	o.pack_array(2);
	const bool is_solid = this->solid_block != nullptr;
	o.pack(is_solid);
	if(is_solid)
	{
		o.pack(this->solid_block);
	}
	else
	{
		o.pack(this->blocks);
	}
}

template<>
void Chunk::load(const msgpack::object& o)
{
	const auto v = o.as<std::vector<msgpack::object>>();
	decltype(v)::size_type i = 0;

	const bool is_solid = v.at(i++).as<bool>();
	if(is_solid)
	{
		this->set_blocks(v.at(i++).as<std::unique_ptr<Block::Base>>());
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
		// let us hope this copy is optimized out
		this->set_blocks(o.as<chunk_blocks_t>());
	}
}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<Chunk>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const Chunk& chunk) const
	{
		chunk.save(o);
		return o;
	}
};

template<>
struct convert<Chunk>
{
	const msgpack::object& operator()(const msgpack::object& o, Chunk& chunk) const
	{
		chunk.load(o);
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
