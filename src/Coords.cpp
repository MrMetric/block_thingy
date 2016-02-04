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
	ChunkInWorld::ChunkInWorld(BlockInWorld bwp)
	{
		this->x = t(bwp.x);
		this->y = t(bwp.y);
		this->z = t(bwp.z);
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
	BlockInWorld::BlockInWorld(const ChunkInWorld& cp, const BlockInChunk& bcp)
	{
		this->x = t(cp.x, bcp.x);
		this->y = t(cp.y, bcp.y);
		this->z = t(cp.z, bcp.z);
	}

	#undef t
	#define t(a) BlockInWorld_type(std::floor(a))
	BlockInWorld::BlockInWorld(const glm::dvec3 vec3)
	{
		this->x = t(vec3.x);
		this->y = t(vec3.y);
		this->z = t(vec3.z);
	}

	BlockInWorld::BlockInWorld(const double x, const double y, const double z)
	{
		this->x = t(x);
		this->y = t(y);
		this->z = t(z);
	}

	BlockInChunk::BlockInChunk() : x(0), y(0), z(0) {}

	BlockInChunk::BlockInChunk(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
	{
		if(x >= CHUNK_SIZE
		|| y >= CHUNK_SIZE
		|| z >= CHUNK_SIZE)
		{
			std::ostringstream ss;
			ss << "Position::BlockInChunk constructed with invalid coordinates: (" << int(x) << "," << int(y) << "," << int(z) << ")";
			throw std::logic_error(ss.str());
		}
		this->x = x;
		this->y = y;
		this->z = z;
	}

	// % can return a negative result, so do it properly here
	// TODO: find out if std::mod works instead
	#undef t
	#define t(a) BlockInChunk_type(a - CHUNK_SIZE * std::floor(a / static_cast<double>(CHUNK_SIZE)))
	BlockInChunk::BlockInChunk(BlockInWorld bwp)
	{
		this->x = t(bwp.x);
		this->y = t(bwp.y);
		this->z = t(bwp.z);
	}


	std::ostream& operator<<(std::ostream& os, const ChunkInWorld& cp)
	{
		os << "(" << cp.x << "," << cp.y << "," << cp.z << ")";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const BlockInWorld& bwp)
	{
		os << "(" << bwp.x << "," << bwp.y << "," << bwp.z << ")";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const BlockInChunk& bcp)
	{
		os << "(" << int(bcp.x) << "," << int(bcp.y) << "," << int(bcp.z) << ")";
		return os;
	}

	BlockInWorld& operator+(BlockInWorld& bwp, const glm::ivec3& vec3)
	{
		bwp.x += vec3.x;
		bwp.y += vec3.y;
		bwp.z += vec3.z;
		return bwp;
	}

	bool operator==(const BlockInWorld& bwp1, const BlockInWorld& bwp2)
	{
		return (bwp1.x == bwp2.x) && (bwp1.y == bwp2.y) && (bwp1.z == bwp2.z);
	}
}