#pragma once

#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>

using BlockInWorld_type = int_fast64_t;

namespace Position
{
	struct BlockInChunk;
	struct ChunkInWorld;

	struct BlockInWorld
	{
		BlockInWorld();
		BlockInWorld(BlockInWorld_type x, BlockInWorld_type y, BlockInWorld_type z);
		BlockInWorld(const ChunkInWorld&, const BlockInChunk&);
		explicit BlockInWorld(const glm::dvec3&);
		BlockInWorld(double x, double y, double z);

		BlockInWorld_type operator[](uint_fast8_t) const;
		BlockInWorld_type& operator[](uint_fast8_t);
		BlockInWorld& operator+=(const BlockInWorld&);
		bool operator==(const BlockInWorld&) const;

		// WARNING: due to limited float range, do not use this for large values
		operator glm::vec3() const;

		BlockInWorld_type x, y, z;
	};

	BlockInWorld operator+(const BlockInWorld&, const glm::ivec3&);

	std::ostream& operator<<(std::ostream&, const BlockInWorld&);
}
