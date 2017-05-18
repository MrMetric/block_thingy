#include "Air.hpp"

#include "block/Enum/Type.hpp"
#include "block/Enum/VisibilityType.hpp"

namespace Block {

Air::Air(Enum::Type t)
:
	Base(t)
{
}

Enum::VisibilityType Air::visibility_type() const
{
	return Enum::VisibilityType::invisible;
}

bool Air::is_solid() const
{
	return false;
}

bool Air::is_selectable() const
{
	return false;
}

bool Air::is_replaceable() const
{
	return true;
}

}
