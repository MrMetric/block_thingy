#include "ChunkInWorld.hpp"

#include <cmath>
#include <iostream>

#include "BlockInWorld.hpp"
#include "fwd/chunk/Chunk.hpp"

namespace Position {

ChunkInWorld::ChunkInWorld()
:
	x(0),
	y(0),
	z(0)
{
}

ChunkInWorld::ChunkInWorld(const value_type x, const value_type y, const value_type z)
:
	x(x),
	y(y),
	z(z)
{
}

#define t(a) static_cast<value_type>(std::floor(a / static_cast<double>(CHUNK_SIZE)))
ChunkInWorld::ChunkInWorld(const BlockInWorld& pos)
{
	x = t(pos.x);
	y = t(pos.y);
	z = t(pos.z);
}
#undef t

ChunkInWorld& ChunkInWorld::operator+=(const ChunkInWorld& that)
{
	x += that.x;
	y += that.y;
	z += that.z;
	return *this;
}

bool ChunkInWorld::operator==(const ChunkInWorld& that) const
{
	return (x == that.x) && (y == that.y) && (z == that.z);
}

bool ChunkInWorld::operator!=(const ChunkInWorld& that) const
{
	return !(*this == that);
}

ChunkInWorld::operator ChunkInWorld::vec_type() const
{
	return {x, y, z};
}

ChunkInWorld operator-(const ChunkInWorld& pos, const ChunkInWorld::value_type a)
{
	return
	{
		pos.x - a,
		pos.y - a,
		pos.z - a,
	};
}

ChunkInWorld operator+(const ChunkInWorld& pos, const ChunkInWorld::value_type a)
{
	return
	{
		pos.x + a,
		pos.y + a,
		pos.z + a,
	};
}

ChunkInWorld operator*(const ChunkInWorld& pos, const ChunkInWorld::value_type a)
{
	return
	{
		pos.x * a,
		pos.y * a,
		pos.z * a,
	};
}

ChunkInWorld operator-(const ChunkInWorld& pos1, const ChunkInWorld& pos2)
{
	return
	{
		pos1.x - pos2.x,
		pos1.y - pos2.y,
		pos1.z - pos2.z,
	};
}

ChunkInWorld operator+(const ChunkInWorld& pos1, const ChunkInWorld& pos2)
{
	return
	{
		pos1.x + pos2.x,
		pos1.y + pos2.y,
		pos1.z + pos2.z,
	};
}

std::ostream& operator<<(std::ostream& os, const ChunkInWorld& pos)
{
	return os << '(' << pos.x << ',' << pos.y << ',' << pos.z << ')';
}

}
