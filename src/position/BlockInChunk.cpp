#include "BlockInChunk.hpp"

#include <ostream>
#include <sstream>

#ifdef DEBUG_BUILD
	#include <stdexcept>
#endif

#include <glm/common.hpp>

#include "BlockInWorld.hpp"
#include "fwd/chunk/Chunk.hpp"

namespace Position {

BlockInChunk::BlockInChunk()
:
	x(0),
	y(0),
	z(0)
{
}

BlockInChunk::BlockInChunk(const value_type x, const value_type y, const value_type z)
:
	x(x),
	y(y),
	z(z)
{
#ifdef DEBUG_BUILD
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
BlockInChunk::BlockInChunk(const BlockInWorld& pos)
{
	x = t(pos.x);
	y = t(pos.y);
	z = t(pos.z);
}

BlockInChunk& BlockInChunk::operator+=(const BlockInChunk& that)
{
	x += that.x;
	y += that.y;
	z += that.z;
#ifdef DEBUG_BUILD
	check_bounds();
#endif
	return *this;
}

bool BlockInChunk::operator==(const BlockInChunk& that) const
{
	return (x == that.x) && (y == that.y) && (z == that.z);
}

bool BlockInChunk::operator!=(const BlockInChunk& that) const
{
	return !(*this == that);
}

BlockInChunk::operator BlockInChunk::vec_type() const
{
	return {x, y, z};
}

#ifdef DEBUG_BUILD
void BlockInChunk::check_bounds()
{
	if(x >= CHUNK_SIZE
	|| y >= CHUNK_SIZE
	|| z >= CHUNK_SIZE)
	{
		std::ostringstream ss;
		ss << "Position::BlockInChunk has invalid coordinates: (" << +x << ',' << +y << ',' << +z << ')';
		throw std::logic_error(ss.str());
	}
}
#endif

std::ostream& operator<<(std::ostream& o, const BlockInChunk& pos)
{
	return o << '(' << +pos.x << ',' << +pos.y << ',' << +pos.z << ')';
}

}
