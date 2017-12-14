#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdint.h>

#ifdef DEBUG_BUILD
	#include <stdexcept>
	#include <string>
#endif

#include <glm/vec3.hpp>

#include "fwd/position/BlockInWorld.hpp"

namespace block_thingy::position {

struct ChunkInWorld
{
	using value_type = int64_t;
	using vec_type = glm::tvec3<value_type>;

	ChunkInWorld();
	ChunkInWorld(value_type x, value_type y, value_type z);
	explicit ChunkInWorld(const BlockInWorld&);

	value_type operator[](const std::ptrdiff_t i) const
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::ChunkInWorld::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	value_type& operator[](const std::ptrdiff_t i)
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::ChunkInWorld::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	ChunkInWorld& operator+=(const ChunkInWorld&);
	bool operator==(const ChunkInWorld&) const;
	bool operator!=(const ChunkInWorld&) const;

	operator vec_type() const;

	value_type x, y, z;
};

ChunkInWorld operator-(const ChunkInWorld&, ChunkInWorld::value_type);
ChunkInWorld operator+(const ChunkInWorld&, ChunkInWorld::value_type);
ChunkInWorld operator*(const ChunkInWorld&, ChunkInWorld::value_type);

ChunkInWorld operator-(const ChunkInWorld&, const ChunkInWorld&);
ChunkInWorld operator+(const ChunkInWorld&, const ChunkInWorld&);

std::ostream& operator<<(std::ostream&, const ChunkInWorld&);

}
