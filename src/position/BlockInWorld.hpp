#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdint.h>

#ifdef DEBUG_BUILD
	#include <stdexcept>
	#include <string>
#endif

#include <glm/vec3.hpp>

#include "fwd/position/BlockInChunk.hpp"
#include "fwd/position/ChunkInWorld.hpp"

namespace block_thingy::position {

struct BlockInWorld
{
	using value_type = int_fast64_t;
	using vec_type = glm::tvec3<value_type>;

	BlockInWorld();
	BlockInWorld(value_type x, value_type y, value_type z);
	BlockInWorld(const ChunkInWorld&, const BlockInChunk&);
	explicit BlockInWorld(const glm::dvec3&);

	value_type operator[](const std::ptrdiff_t i) const
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::BlockInWorld::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	value_type& operator[](const std::ptrdiff_t i)
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::BlockInWorld::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	BlockInWorld& operator+=(const BlockInWorld&);
	bool operator==(const BlockInWorld&) const;
	bool operator!=(const BlockInWorld&) const;

	operator vec_type() const;

	value_type x, y, z;
};

BlockInWorld operator+(const BlockInWorld&, const BlockInWorld&);
BlockInWorld operator+(const BlockInWorld&, const glm::ivec3&);

std::ostream& operator<<(std::ostream&, const BlockInWorld&);

}
