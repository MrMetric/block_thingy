#pragma once

#include <iosfwd>
#include <stdint.h>

#include "fwd/position/BlockInWorld.hpp"

namespace Position
{
	struct BlockInChunk
	{
		using value_type = uint8_t;

		BlockInChunk();
		BlockInChunk(value_type x, value_type y, value_type z);
		explicit BlockInChunk(const BlockInWorld&);

		value_type operator[](uint_fast8_t) const;
		value_type& operator[](uint_fast8_t);
		BlockInChunk& operator+=(const BlockInChunk&);
		bool operator==(const BlockInChunk&) const;

		void check_bounds();

		value_type x, y, z;
	};

	std::ostream& operator<<(std::ostream&, const BlockInChunk&);
}
