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
		ChunkInWorld(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z);
		explicit ChunkInWorld(BlockInWorld bwp);

		ChunkInWorld_type x, y, z;
	};

	struct BlockInWorld
	{
		BlockInWorld();
		BlockInWorld(const BlockInWorld_type x, const BlockInWorld_type y, const BlockInWorld_type z);
		BlockInWorld(const ChunkInWorld& cp, const BlockInChunk& bcp);
		explicit BlockInWorld(const glm::dvec3 vec3);
		BlockInWorld(const double x, const double y, const double z);

		BlockInWorld_type operator[](uint_fast8_t i);

		BlockInWorld_type x, y, z;
	};

	struct BlockInChunk
	{
		BlockInChunk();
		BlockInChunk(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z);
		explicit BlockInChunk(BlockInWorld bwp);

		BlockInChunk_type x, y, z;
	};

	std::ostream& operator<<(std::ostream& os, const ChunkInWorld& cp);
	std::ostream& operator<<(std::ostream& os, const BlockInWorld& cp);
	std::ostream& operator<<(std::ostream& os, const BlockInChunk& cp);
	BlockInWorld& operator+(BlockInWorld& bwp, const glm::ivec3& vec3);
	bool operator==(const BlockInWorld& bwp1, const BlockInWorld& bwp2);
}