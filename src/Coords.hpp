#pragma once

#include <cstdint>
#include <iosfwd>

#include <glm/vec3.hpp>

using bwp_type = int_fast64_t;
using bcp_type = uint8_t;

namespace Position
{
	struct BlockInWorld;
	struct BlockInChunk;

	struct ChunkInWorld
	{
		ChunkInWorld(int32_t x, int32_t y, int32_t z);
		explicit ChunkInWorld(BlockInWorld bwp);

		int32_t x, y, z;
	};

	struct BlockInWorld
	{
		BlockInWorld(const bwp_type x, const bwp_type y, const bwp_type z);
		BlockInWorld(const ChunkInWorld& cp, const BlockInChunk& bcp);
		explicit BlockInWorld(const glm::dvec3 vec3);
		BlockInWorld(const double x, const double y, const double z);

		bwp_type x, y, z;
	};

	struct BlockInChunk
	{
		BlockInChunk(const bcp_type x, const bcp_type y, const bcp_type z);
		explicit BlockInChunk(BlockInWorld bwp);

		bcp_type x, y, z;
	};

	std::ostream& operator<<(std::ostream& os, const ChunkInWorld& cp);
	std::ostream& operator<<(std::ostream& os, const BlockInWorld& cp);
	std::ostream& operator<<(std::ostream& os, const BlockInChunk& cp);
	BlockInWorld& operator+(BlockInWorld& bwp, const glm::ivec3& vec3);
	bool operator==(const BlockInWorld& bwp1, const BlockInWorld& bwp2);
}