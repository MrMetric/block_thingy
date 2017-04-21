#pragma once

#include <iosfwd>
#include <stdint.h>

namespace Block::Enum {

enum class Face : uint8_t
{
	front = 0,
	back = 1,
	top = 2,
	bottom = 3,
	right = 4,
	left = 5,
};

std::ostream& operator<<(std::ostream&, Block::Enum::Face);

}
