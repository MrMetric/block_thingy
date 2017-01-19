#pragma once

#include <vector>

#include "chunk/Chunk.hpp"
#include "storage/msgpack/Block.hpp"
#include "storage/msgpack/Color.hpp" // for meshes
#include "storage/msgpack/glm_vec4.hpp" // for meshes

#include "std_make_unique.hpp"

template<>
void Chunk::save(msgpack::packer<zstr::ostream>& o) const
{
	const bool has_meshes = false;
	uint32_t array_size = 3;
	if(has_meshes) array_size += 1;

	o.pack_array(array_size);
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

	o.pack(has_meshes);
	if(has_meshes)
	{
		o.pack(this->get_meshes());
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
		// let us hope this copy is optimized out
		this->set_blocks(v.at(i++).as<chunk_block_array_t>());
	}

	const bool has_meshes = v.at(i++).as<bool>();
	if(has_meshes)
	{
		this->set_meshes(v.at(i++).as<Mesher::meshmap_t>());
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
