#pragma once

#include <iosfwd>
#include <stdint.h>

namespace Block::Enum {

enum class Face : uint8_t
{
	right  = 0, // +x
	left   = 1, // -x
	top    = 2, // +y
	bottom = 3, // -y
	front  = 4, // +z
	back   = 5, // -z
};

std::ostream& operator<<(std::ostream&, Face);

}
