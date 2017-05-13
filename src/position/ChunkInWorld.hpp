#pragma once

#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>

#include "fwd/position/BlockInWorld.hpp"

namespace Position {

struct ChunkInWorld
{
	using value_type = int64_t;
	using vec_type = glm::tvec3<value_type>;

	ChunkInWorld();
	ChunkInWorld(value_type x, value_type y, value_type z);
	explicit ChunkInWorld(const BlockInWorld&);

	value_type operator[](uint_fast8_t) const;
	value_type& operator[](uint_fast8_t);
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
