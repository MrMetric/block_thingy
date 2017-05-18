#pragma once

#include <iosfwd>
#include <stdint.h>

namespace Block::Enum {

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

}
