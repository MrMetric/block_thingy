#pragma once

#include <iosfwd>
#include <stdint.h>

namespace block_thingy::block::enums {

using type_t = uint16_t;

enum class type : type_t
{
	none,
	air,
	unknown,
};
std::ostream& operator<<(std::ostream&, type);

enum class type_external : type_t
{
};
std::ostream& operator<<(std::ostream&, type_external);

}
