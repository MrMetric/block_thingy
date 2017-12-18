#pragma once

#include <iosfwd>
#include <stdint.h>

namespace block_thingy::block::enums {

enum class visibility_type : uint8_t
{
	opaque,
	translucent,
	invisible,
};
std::ostream& operator<<(std::ostream&, visibility_type);

}
