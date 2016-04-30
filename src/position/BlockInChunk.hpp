#pragma once

#include <iosfwd>
#include <stdint.h>

using BlockInChunk_type = uint8_t;

namespace Position
{
	struct BlockInWorld;

	struct BlockInChunk
	{
		BlockInChunk();
		BlockInChunk(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z);
		explicit BlockInChunk(const BlockInWorld&);

		BlockInChunk_type operator[](uint_fast8_t) const;
		BlockInChunk_type& operator[](uint_fast8_t);
		BlockInChunk& operator+=(const BlockInChunk&);
		bool operator==(const BlockInChunk&) const;

		void check_bounds();

		BlockInChunk_type x, y, z;
	};

	std::ostream& operator<<(std::ostream&, const BlockInChunk&);
}
