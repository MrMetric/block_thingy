#include "ChunkInWorld.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include "BlockInWorld.hpp"

#include "chunk/Chunk.hpp"

using std::to_string;

namespace Position
{
	ChunkInWorld::ChunkInWorld() : x(0), y(0), z(0) {}

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

	ChunkInWorld::value_type& ChunkInWorld::operator[](const uint_fast8_t i)
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("Position::ChunkInWorld::operator[]: " + to_string(i) + " > 2");
	}

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

	ChunkInWorld operator-(const ChunkInWorld& pos, const ChunkInWorld::value_type a)
	{
		return ChunkInWorld(pos.x - a, pos.y - a, pos.z - a);
	}

	ChunkInWorld operator+(const ChunkInWorld& pos, const ChunkInWorld::value_type a)
	{
		return ChunkInWorld(pos.x + a, pos.y + a, pos.z + a);
	}

	ChunkInWorld operator*(const ChunkInWorld& pos, const ChunkInWorld::value_type a)
	{
		return ChunkInWorld(pos.x * a, pos.y * a, pos.z * a);
	}

	ChunkInWorld operator-(const ChunkInWorld& pos1, const ChunkInWorld& pos2)
	{
		return ChunkInWorld(pos1.x - pos2.x, pos1.y - pos2.y, pos1.z - pos2.z);
	}

	std::ostream& operator<<(std::ostream& os, const ChunkInWorld& pos)
	{
		return os << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
	}
}
