#include "Coords.hpp"

#include <cmath> // floor
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <glm/vec3.hpp>

#include "chunk/Chunk.hpp"

namespace Position
{
	ChunkInWorld::ChunkInWorld() : x(0), y(0), z(0) {}

	ChunkInWorld::ChunkInWorld(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z)
		:
		x(x),
		y(y),
		z(z)
	{
	}

	#define t(a) ChunkInWorld_type(std::floor(a / static_cast<double>(CHUNK_SIZE)))
	ChunkInWorld::ChunkInWorld(const BlockInWorld& pos)
	{
		x = t(pos.x);
		y = t(pos.y);
		z = t(pos.z);
	}

	ChunkInWorld_type ChunkInWorld::operator[](uint_fast8_t i)
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("ChunkInWorld::operator[]: " + std::to_string(i) + " > 2");
	}

	ChunkInWorld& ChunkInWorld::operator+=(const ChunkInWorld& that)
	{
		x += that.x;
		y += that.y;
		z += that.z;
		return *this;
	}

	ChunkInWorld operator-(const ChunkInWorld& chunk_pos, const ChunkInWorld_type a)
	{
		return ChunkInWorld(chunk_pos.x - a, chunk_pos.y - a, chunk_pos.z - a);
	}

	ChunkInWorld operator+(const ChunkInWorld& chunk_pos, const ChunkInWorld_type a)
	{
		return ChunkInWorld(chunk_pos.x + a, chunk_pos.y + a, chunk_pos.z + a);
	}

	BlockInWorld::BlockInWorld() : x(0), y(0), z(0) {}

	BlockInWorld::BlockInWorld(const BlockInWorld_type x, const BlockInWorld_type y, const BlockInWorld_type z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	#undef t
	#define t(a,b) static_cast<BlockInWorld_type>(a) * CHUNK_SIZE + b
	BlockInWorld::BlockInWorld(const ChunkInWorld& chunk_pos, const BlockInChunk& block_pos)
	{
		x = t(chunk_pos.x, block_pos.x);
		y = t(chunk_pos.y, block_pos.y);
		z = t(chunk_pos.z, block_pos.z);
	}

	#undef t
	#define t(a) BlockInWorld_type(std::floor(a))
	BlockInWorld::BlockInWorld(const glm::dvec3& vec)
	{
		x = t(vec.x);
		y = t(vec.y);
		z = t(vec.z);
	}

	BlockInWorld::BlockInWorld(const double x, const double y, const double z)
	{
		this->x = t(x);
		this->y = t(y);
		this->z = t(z);
	}

	BlockInWorld_type BlockInWorld::operator[](uint_fast8_t i)
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("BlockInWorld::operator[]: " + std::to_string(i) + " > 2");
	}

	BlockInWorld& BlockInWorld::operator+=(const BlockInWorld& that)
	{
		x += that.x;
		y += that.y;
		z += that.z;
		return *this;
	}

	BlockInWorld operator+(const BlockInWorld& pos, const glm::ivec3& vec)
	{
		return Position::BlockInWorld(pos.x + vec.x, pos.y + vec.y, pos.z + vec.z);
	}

	BlockInChunk::BlockInChunk() : x(0), y(0), z(0) {}

	BlockInChunk::BlockInChunk(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		check_bounds();
	}

	// % can return a negative result, so do it properly here
	// TODO: find out if std::mod works instead
	#undef t
	#define t(a) BlockInChunk_type(a - CHUNK_SIZE * std::floor(a / static_cast<double>(CHUNK_SIZE)))
	BlockInChunk::BlockInChunk(const BlockInWorld& pos)
	{
		x = t(pos.x);
		y = t(pos.y);
		z = t(pos.z);
	}

	BlockInChunk_type BlockInChunk::operator[](uint_fast8_t i)
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("BlockInChunk::operator[]: " + std::to_string(i) + " > 2");
	}

	BlockInChunk& BlockInChunk::operator+=(const BlockInChunk& that)
	{
		x += that.x;
		y += that.y;
		z += that.z;
		check_bounds();
		return *this;
	}

	void BlockInChunk::check_bounds()
	{
		if(x >= CHUNK_SIZE
		|| y >= CHUNK_SIZE
		|| z >= CHUNK_SIZE)
		{
			std::ostringstream ss;
			ss << "Position::BlockInChunk has invalid coordinates: (" << int(x) << "," << int(y) << "," << int(z) << ")";
			throw std::logic_error(ss.str());
		}
	}


	std::ostream& operator<<(std::ostream& os, const ChunkInWorld& pos)
	{
		os << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const BlockInWorld& pos)
	{
		os << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const BlockInChunk& pos)
	{
		os << "(" << int(pos.x) << "," << int(pos.y) << "," << int(pos.z) << ")";
		return os;
	}

	bool operator==(const BlockInWorld& pos1, const BlockInWorld& pos2)
	{
		return (pos1.x == pos2.x) && (pos1.y == pos2.y) && (pos1.z == pos2.z);
	}
}
