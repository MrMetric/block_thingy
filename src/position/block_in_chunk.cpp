#include "block_in_chunk.hpp"

#include <ostream>
#include <sstream>

#ifdef BT_DEBUG_BUILD
	#include <stdexcept>
#endif

#include <glm/common.hpp>

#include "fwd/chunk/Chunk.hpp"
#include "position/block_in_world.hpp"

namespace block_thingy::position {

block_in_chunk::block_in_chunk()
:
	x(0),
	y(0),
	z(0)
{
}

block_in_chunk::block_in_chunk(const value_type x, const value_type y, const value_type z)
:
	x(x),
	y(y),
	z(z)
{
#ifdef BT_DEBUG_BUILD
	check_bounds();
#endif
}

/*
+5 % +2 == std::fmod(+5, +2) == 1
+5 % -2 == std::fmod(+5, -2) == 1
-5 % +2 == std::fmod(-5, +2) == -1
-5 % -2 == std::fmod(-5, -2) == -1
glm::mod(+5.0, +2.0) == 1
glm::mod(+5.0, -2.0) == -1
glm::mod(-5.0, +2.0) == 1
glm::mod(-5.0, -2.0) == -1
*/
#undef t
#define t(a) static_cast<value_type>(glm::mod(static_cast<double>(a), static_cast<double>(CHUNK_SIZE)))
block_in_chunk::block_in_chunk(const block_in_world& pos)
{
	x = t(pos.x);
	y = t(pos.y);
	z = t(pos.z);
}

block_in_chunk& block_in_chunk::operator+=(const block_in_chunk& that)
{
	x += that.x;
	y += that.y;
	z += that.z;
#ifdef BT_DEBUG_BUILD
	check_bounds();
#endif
	return *this;
}

bool block_in_chunk::operator==(const block_in_chunk& that) const
{
	return (x == that.x) && (y == that.y) && (z == that.z);
}

bool block_in_chunk::operator!=(const block_in_chunk& that) const
{
	return !(*this == that);
}

block_in_chunk::operator block_in_chunk::vec_type() const
{
	return {x, y, z};
}

#ifdef BT_DEBUG_BUILD
void block_in_chunk::check_bounds()
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		std::ostringstream ss;
		ss << "position::block_in_chunk has invalid coordinates: (" << +x << ',' << +y << ',' << +z << ')';
		throw std::logic_error(ss.str());
	}
}
#endif

std::ostream& operator<<(std::ostream& o, const block_in_chunk& pos)
{
	return o << '(' << +pos.x << ',' << +pos.y << ',' << +pos.z << ')';
}

}
