#include "BlockInWorld.hpp"

#include <cmath>
#include <ostream>

#include "BlockInChunk.hpp"
#include "ChunkInWorld.hpp"
#include "chunk/Chunk.hpp"

namespace block_thingy::position {

BlockInWorld::BlockInWorld()
:
	x(0),
	y(0),
	z(0)
{
}

BlockInWorld::BlockInWorld(const value_type x, const value_type y, const value_type z)
:
	x(x),
	y(y),
	z(z)
{
}

#undef t
#define t(a, b) static_cast<value_type>(a) * CHUNK_SIZE + b
BlockInWorld::BlockInWorld(const ChunkInWorld& chunk_pos, const BlockInChunk& block_pos)
{
	x = t(chunk_pos.x, block_pos.x);
	y = t(chunk_pos.y, block_pos.y);
	z = t(chunk_pos.z, block_pos.z);
}

#undef t
#define t(a) static_cast<value_type>(std::floor(a))
BlockInWorld::BlockInWorld(const glm::dvec3& vec)
{
	x = t(vec.x);
	y = t(vec.y);
	z = t(vec.z);
}
#undef t

BlockInWorld& BlockInWorld::operator+=(const BlockInWorld& that)
{
	x += that.x;
	y += that.y;
	z += that.z;
	return *this;
}

bool BlockInWorld::operator==(const BlockInWorld& that) const
{
	return (x == that.x) && (y == that.y) && (z == that.z);
}

bool BlockInWorld::operator!=(const BlockInWorld& that) const
{
	return !(*this == that);
}

BlockInWorld::operator BlockInWorld::vec_type() const
{
	return {x, y, z};
}

BlockInWorld operator+(const BlockInWorld& pos1, const BlockInWorld& pos2)
{
	return {pos1.x + pos2.x, pos1.y + pos2.y, pos1.z + pos2.z};
}

BlockInWorld operator+(const BlockInWorld& pos, const glm::ivec3& vec)
{
	return BlockInWorld(pos.x + vec.x, pos.y + vec.y, pos.z + vec.z);
}

std::ostream& operator<<(std::ostream& o, const BlockInWorld& pos)
{
	return o << '(' << pos.x << ',' << pos.y << ',' << pos.z << ')';
}

}
