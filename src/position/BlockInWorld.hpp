#pragma once

#include <cstdint>
#include <iosfwd>

#include <glm/vec3.hpp>

using BlockInWorld_type = int_fast64_t;

namespace Position
{
	struct BlockInChunk;
	struct ChunkInWorld;

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
	bool operator==(const BlockInWorld&, const BlockInWorld&);

	std::ostream& operator<<(std::ostream&, const BlockInWorld&);
}
