#pragma once

#include <iosfwd>
#include <stdint.h>

namespace block_thingy::block::enums {

enum class VisibilityType : uint8_t
{
	opaque,
	translucent,
	invisible,
};
std::ostream& operator<<(std::ostream&, VisibilityType);

}
