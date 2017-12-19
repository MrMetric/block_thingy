#include "block_in_world.hpp"

#include <cmath>
#include <ostream>

#include "chunk/Chunk.hpp"
#include "position/block_in_chunk.hpp"
#include "position/chunk_in_world.hpp"

namespace block_thingy::position {

block_in_world::block_in_world()
:
	x(0),
	y(0),
	z(0)
{
}

block_in_world::block_in_world(const value_type x, const value_type y, const value_type z)
:
	x(x),
	y(y),
	z(z)
{
}

#undef t
#define t(a, b) static_cast<value_type>(a) * CHUNK_SIZE + b
block_in_world::block_in_world(const chunk_in_world& chunk_pos, const block_in_chunk& block_pos)
{
	x = t(chunk_pos.x, block_pos.x);
	y = t(chunk_pos.y, block_pos.y);
	z = t(chunk_pos.z, block_pos.z);
}

#undef t
#define t(a) static_cast<value_type>(std::floor(a))
block_in_world::block_in_world(const glm::dvec3& vec)
{
	x = t(vec.x);
	y = t(vec.y);
	z = t(vec.z);
}
#undef t

block_in_world& block_in_world::operator+=(const block_in_world& that)
{
	x += that.x;
	y += that.y;
	z += that.z;
	return *this;
}

bool block_in_world::operator==(const block_in_world& that) const
{
	return (x == that.x) && (y == that.y) && (z == that.z);
}

bool block_in_world::operator!=(const block_in_world& that) const
{
	return !(*this == that);
}

block_in_world::operator block_in_world::vec_type() const
{
	return {x, y, z};
}

block_in_world operator+(const block_in_world& pos1, const block_in_world& pos2)
{
	return {pos1.x + pos2.x, pos1.y + pos2.y, pos1.z + pos2.z};
}

block_in_world operator+(const block_in_world& pos, const glm::ivec3& vec)
{
	return block_in_world(pos.x + vec.x, pos.y + vec.y, pos.z + vec.z);
}

std::ostream& operator<<(std::ostream& o, const block_in_world& pos)
{
	return o << '(' << pos.x << ',' << pos.y << ',' << pos.z << ')';
}

}
