#pragma once

#include <iosfwd>
#include <stdint.h>

namespace Block::Enum {

enum class VisibilityType : uint8_t
{
	opaque,
	translucent,
	invisible,
};
std::ostream& operator<<(std::ostream&, VisibilityType);

}
