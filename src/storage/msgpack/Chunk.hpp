#pragma once

#include "chunk/Chunk.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<Chunk>
{
	template <typename Stream>
	packer<Stream>& operator()(msgpack::packer<Stream>& o, Chunk const& chunk) const
	{
		const bool has_meshes = false;
		uint32_t array_size = 3;
		if(has_meshes) array_size += 1;

		o.pack_array(array_size);
		const bool is_solid = chunk.blocks == nullptr;
		o.pack(is_solid);
		if(is_solid)
		{
			o.pack(chunk.solid_block);
		}
		else
		{
			o.pack(chunk.blocks);
		}

		o.pack(has_meshes);
		if(has_meshes)
		{
			o.pack(chunk.get_meshes());
		}

		return o;
	}
};

template<>
struct convert<Chunk>
{
	msgpack::object const& operator()(msgpack::object const& o, Chunk& chunk) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size < 3) throw msgpack::type_error();

		auto array = o.via.array.ptr;
		uint_fast8_t i = 0;

		const bool is_solid = array[i++].as<bool>();
		if(is_solid)
		{
			chunk.set_blocks(array[i++].as<std::unique_ptr<Block::Base>>());
		}
		else
		{
			// let us hope this copy is optimized out
			chunk.set_blocks(std::make_unique<chunk_block_array_t>(array[i++].as<chunk_block_array_t>()));
		}

		const bool has_meshes = array[i++].as<bool>();
		if(has_meshes)
		{
			chunk.set_meshes(array[i++].as<meshmap_t>());
		}

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
