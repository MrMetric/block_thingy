#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

#include <msgpack/object_fwd_decl.hpp>
#include <msgpack/pack_decl.hpp>

#include "block/block.hpp"

namespace block_thingy::block::component {


class base
{
public:
	virtual ~base();

	virtual std::string get_id() const = 0;

	virtual void copy(block_t in, block_t out) = 0;
	virtual void copy(const block_t* in, const block_t* out, std::size_t N) = 0;

	virtual void save(msgpack::packer<std::ofstream>&) const = 0;
	virtual void load(const msgpack::object&) = 0;
};

}
