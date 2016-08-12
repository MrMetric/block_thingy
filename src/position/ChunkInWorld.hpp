#pragma once

#include <iosfwd>
#include <stdint.h>

namespace Position
{
	struct BlockInWorld;

	struct ChunkInWorld
	{
		using value_type = int64_t;

		ChunkInWorld();
		ChunkInWorld(value_type x, value_type y, value_type z);
		explicit ChunkInWorld(const BlockInWorld&);

		value_type operator[](uint_fast8_t) const;
		value_type& operator[](uint_fast8_t);
		ChunkInWorld& operator+=(const ChunkInWorld&);
		bool operator==(const ChunkInWorld&) const;
		bool operator<(const ChunkInWorld&) const;

		value_type x, y, z;
	};

	ChunkInWorld operator-(const ChunkInWorld&, ChunkInWorld::value_type);
	ChunkInWorld operator+(const ChunkInWorld&, ChunkInWorld::value_type);
	ChunkInWorld operator*(const ChunkInWorld&, ChunkInWorld::value_type);

	ChunkInWorld operator-(const ChunkInWorld&, const ChunkInWorld&);

	std::ostream& operator<<(std::ostream&, const ChunkInWorld&);
}
