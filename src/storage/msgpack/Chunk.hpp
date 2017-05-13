#pragma once

#include <vector>

#include "chunk/Chunk.hpp"
#include "chunk/ChunkData.hpp"
#include "storage/msgpack/Block.hpp"
#include "storage/msgpack/ChunkData.hpp"

template<>
void Chunk::save(msgpack::packer<zstr::ostream>& o) const
{
	o.pack(this->blocks);
}

template<>
void Chunk::load(const msgpack::object& o)
{
	this->set_blocks(o.as<ChunkData<std::unique_ptr<Block::Base>>>());
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
