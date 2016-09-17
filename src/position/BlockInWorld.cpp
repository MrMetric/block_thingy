#include "BlockInWorld.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include "BlockInChunk.hpp"
#include "ChunkInWorld.hpp"

#include "chunk/Chunk.hpp"

using std::to_string;

namespace Position {

BlockInWorld::BlockInWorld()
:
	x(0),
	y(0),
	z(0)
{
}

BlockInWorld::BlockInWorld(const value_type x, const value_type y, const value_type z)
{
	this->x = x;
	this->y = y;
	this->z = z;
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

BlockInWorld::value_type BlockInWorld::operator[](const uint_fast8_t i) const
{
	if(i == 0) return x;
	if(i == 1) return y;
	if(i == 2) return z;
	throw std::out_of_range("Position::BlockInWorld::operator[]: " + to_string(i) + " > 2");
}

BlockInWorld::value_type& BlockInWorld::operator[](const uint_fast8_t i)
{
	if(i == 0) return x;
	if(i == 1) return y;
	if(i == 2) return z;
	throw std::out_of_range("Position::BlockInWorld::operator[]: " + to_string(i) + " > 2");
}

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

BlockInWorld::operator glm::vec3() const
{
	return glm::vec3(x, y, z);
}

BlockInWorld operator+(const BlockInWorld& pos1, const BlockInWorld& pos2)
{
	return {pos1.x + pos2.x, pos1.y + pos2.y, pos1.z + pos2.z};
}

BlockInWorld operator+(const BlockInWorld& pos, const glm::ivec3& vec)
{
	return Position::BlockInWorld(pos.x + vec.x, pos.y + vec.y, pos.z + vec.z);
}

std::ostream& operator<<(std::ostream& os, const BlockInWorld& pos)
{
	return os << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
}

}
