#pragma once

#include <vector>

#include "chunk/Chunk.hpp"
#include "chunk/ChunkData.hpp"
#include "storage/msgpack/block.hpp"
#include "storage/msgpack/ChunkData.hpp"
#include "storage/msgpack/color.hpp"

namespace block_thingy {

template<>
void Chunk::save(msgpack::packer<zstr::ostream>& o) const
{
	o.pack_array(3);
	o.pack(blocks);
	o.pack(blocklight);
	o.pack(skylight);
}

template<>
void Chunk::load(const msgpack::object& o)
{
	if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
	if(o.via.array.size != 3) throw msgpack::type_error();

	blocks = o.via.array.ptr[0].as<decltype(blocks)>();
	blocklight = o.via.array.ptr[1].as<decltype(blocklight)>();
	skylight = o.via.array.ptr[2].as<decltype(skylight)>();

	regenerate_texbuflight();
}

}

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::Chunk;

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
