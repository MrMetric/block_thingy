#pragma once

#include <cstdint>
#include <iosfwd>

#include <glm/vec3.hpp>

using ChunkInWorld_type = int32_t;
using BlockInWorld_type = int_fast64_t;
using BlockInChunk_type = uint8_t;

namespace Position
{
	struct BlockInWorld;
	struct BlockInChunk;

	struct ChunkInWorld
	{
		ChunkInWorld();
		ChunkInWorld(const ChunkInWorld_type x, const ChunkInWorld_type y, const ChunkInWorld_type z);
		explicit ChunkInWorld(const BlockInWorld&);

		ChunkInWorld_type& operator[](uint_fast8_t);
		ChunkInWorld& operator+=(const ChunkInWorld&);

		ChunkInWorld_type x, y, z;
	};

	ChunkInWorld operator-(const ChunkInWorld&, const ChunkInWorld_type);
	ChunkInWorld operator+(const ChunkInWorld&, const ChunkInWorld_type);

	struct BlockInWorld
	{
		BlockInWorld();
		BlockInWorld(const BlockInWorld_type x, const BlockInWorld_type y, const BlockInWorld_type z);
		BlockInWorld(const ChunkInWorld&, const BlockInChunk&);
		explicit BlockInWorld(const glm::dvec3&);
		BlockInWorld(const double x, const double y, const double z);

		BlockInWorld_type& operator[](uint_fast8_t);
		BlockInWorld& operator+=(const BlockInWorld&);

		BlockInWorld_type x, y, z;
	};

	BlockInWorld operator+(const BlockInWorld&, const glm::ivec3&);

	struct BlockInChunk
	{
		BlockInChunk();
		BlockInChunk(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z);
		explicit BlockInChunk(const BlockInWorld&);

		BlockInChunk_type& operator[](uint_fast8_t);
		BlockInChunk& operator+=(const BlockInChunk&);

		void check_bounds();

		BlockInChunk_type x, y, z;
	};

	std::ostream& operator<<(std::ostream&, const ChunkInWorld&);
	std::ostream& operator<<(std::ostream&, const BlockInWorld&);
	std::ostream& operator<<(std::ostream&, const BlockInChunk&);
	bool operator==(const BlockInWorld&, const BlockInWorld&);
}
