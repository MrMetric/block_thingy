#include "chunk_in_world.hpp"

#include <cmath>
#include <ostream>

#include "fwd/chunk/Chunk.hpp"
#include "position/block_in_world.hpp"

namespace block_thingy::position {

chunk_in_world::chunk_in_world()
:
	x(0),
	y(0),
	z(0)
{
}

chunk_in_world::chunk_in_world(const value_type x, const value_type y, const value_type z)
:
	x(x),
	y(y),
	z(z)
{
}

#define t(a) static_cast<value_type>(std::floor((a) / static_cast<double>(CHUNK_SIZE)))
chunk_in_world::chunk_in_world(const block_in_world& pos)
{
	x = t(pos.x);
	y = t(pos.y);
	z = t(pos.z);
}
#undef t

chunk_in_world& chunk_in_world::operator+=(const chunk_in_world& that)
{
	x += that.x;
	y += that.y;
	z += that.z;
	return *this;
}

bool chunk_in_world::operator==(const chunk_in_world& that) const
{
	return (x == that.x) && (y == that.y) && (z == that.z);
}

bool chunk_in_world::operator!=(const chunk_in_world& that) const
{
	return !(*this == that);
}

chunk_in_world::operator chunk_in_world::vec_type() const
{
	return {x, y, z};
}

chunk_in_world operator-(const chunk_in_world& pos, const chunk_in_world::value_type a)
{
	return
	{
		pos.x - a,
		pos.y - a,
		pos.z - a,
	};
}

chunk_in_world operator+(const chunk_in_world& pos, const chunk_in_world::value_type a)
{
	return
	{
		pos.x + a,
		pos.y + a,
		pos.z + a,
	};
}

chunk_in_world operator*(const chunk_in_world& pos, const chunk_in_world::value_type a)
{
	return
	{
		pos.x * a,
		pos.y * a,
		pos.z * a,
	};
}

chunk_in_world operator-(const chunk_in_world& pos1, const chunk_in_world& pos2)
{
	return
	{
		pos1.x - pos2.x,
		pos1.y - pos2.y,
		pos1.z - pos2.z,
	};
}

chunk_in_world operator+(const chunk_in_world& pos1, const chunk_in_world& pos2)
{
	return
	{
		pos1.x + pos2.x,
		pos1.y + pos2.y,
		pos1.z + pos2.z,
	};
}

std::ostream& operator<<(std::ostream& o, const chunk_in_world& pos)
{
	return o << '(' << pos.x << ',' << pos.y << ',' << pos.z << ')';
}

}
