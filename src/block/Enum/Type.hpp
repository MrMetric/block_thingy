#pragma once

#include <iosfwd>
#include <stdint.h>

namespace block_thingy::block::enums {

using Type_t = uint16_t;

enum class Type : Type_t
{
	none,
	air,
	unknown,
};
std::ostream& operator<<(std::ostream&, Type);

enum class TypeExternal : Type_t
{
};
std::ostream& operator<<(std::ostream&, TypeExternal);

}
