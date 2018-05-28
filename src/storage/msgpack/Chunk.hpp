#pragma once

#include <vector>

#include "chunk/Chunk.hpp"
#include "chunk/ChunkData.hpp"
#include "storage/msgpack/block.hpp"
#include "storage/msgpack/ChunkData.hpp"

namespace block_thingy {

template<>
void Chunk::save(msgpack::packer<zstr::ostream>& o) const
{
	o.pack(blocks);
}

template<>
void Chunk::load(const msgpack::object& o)
{
	blocks = o.as<decltype(blocks)>();
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
