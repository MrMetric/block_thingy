#pragma once

#include <cstdint>
#include <iosfwd>

using ChunkInWorld_type = int32_t;

namespace Position
{
	struct BlockInWorld;

	struct ChunkInWorld
	{
		ChunkInWorld();
		ChunkInWorld(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z);
		explicit ChunkInWorld(const BlockInWorld&);

		ChunkInWorld_type& operator[](uint_fast8_t);
		ChunkInWorld& operator+=(const ChunkInWorld&);
		bool operator==(const ChunkInWorld&) const;
		bool operator<(const ChunkInWorld&) const;

		ChunkInWorld_type x, y, z;
	};

	ChunkInWorld operator-(const ChunkInWorld&, ChunkInWorld_type);
	ChunkInWorld operator+(const ChunkInWorld&, ChunkInWorld_type);
	ChunkInWorld operator*(const ChunkInWorld&, ChunkInWorld_type);

	std::ostream& operator<<(std::ostream&, const ChunkInWorld&);
}
