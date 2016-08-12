#pragma once

#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>

namespace Position
{
	struct BlockInChunk;
	struct ChunkInWorld;

	struct BlockInWorld
	{
		using value_type = int_fast64_t;

		BlockInWorld();
		BlockInWorld(value_type x, value_type y, value_type z);
		BlockInWorld(const ChunkInWorld&, const BlockInChunk&);
		explicit BlockInWorld(const glm::dvec3&);

		value_type operator[](uint_fast8_t) const;
		value_type& operator[](uint_fast8_t);
		BlockInWorld& operator+=(const BlockInWorld&);
		bool operator==(const BlockInWorld&) const;
		bool operator!=(const BlockInWorld&) const;

		// WARNING: due to limited float range, do not use this for large values
		operator glm::vec3() const;

		value_type x, y, z;
	};

	BlockInWorld operator+(const BlockInWorld&, const glm::ivec3&);

	std::ostream& operator<<(std::ostream&, const BlockInWorld&);
}
