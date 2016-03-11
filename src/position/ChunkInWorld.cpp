#include "ChunkInWorld.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include "BlockInWorld.hpp"
#include "../chunk/Chunk.hpp"

namespace Position
{
	ChunkInWorld::ChunkInWorld() : x(0), y(0), z(0) {}

	ChunkInWorld::ChunkInWorld(const ChunkInWorld_type x, const ChunkInWorld_type y, const ChunkInWorld_type z)
		:
		x(x),
		y(y),
		z(z)
	{
	}

	#define t(a) static_cast<ChunkInWorld_type>(std::floor(a / static_cast<double>(CHUNK_SIZE)))
	ChunkInWorld::ChunkInWorld(const BlockInWorld& pos)
	{
		x = t(pos.x);
		y = t(pos.y);
		z = t(pos.z);
	}

	ChunkInWorld_type& ChunkInWorld::operator[](const uint_fast8_t i)
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("Position::ChunkInWorld::operator[]: " + std::to_string(i) + " > 2");
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

	ChunkInWorld operator-(const ChunkInWorld& pos, const ChunkInWorld_type a)
	{
		return ChunkInWorld(pos.x - a, pos.y - a, pos.z - a);
	}

	ChunkInWorld operator+(const ChunkInWorld& pos, const ChunkInWorld_type a)
	{
		return ChunkInWorld(pos.x + a, pos.y + a, pos.z + a);
	}

	ChunkInWorld operator*(const ChunkInWorld& pos, const ChunkInWorld_type a)
	{
		return ChunkInWorld(pos.x * a, pos.y * a, pos.z * a);
	}

	std::ostream& operator<<(std::ostream& os, const ChunkInWorld& pos)
	{
		os << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
		return os;
	}
}
