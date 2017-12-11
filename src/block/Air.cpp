#include "Air.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace Block {

Air::Air(const Enum::Type t)
:
	Base(t, Enum::VisibilityType::invisible)
{
}

bool Air::is_solid() const
{
	return false;
}

bool Air::is_selectable() const
{
	return false;
}

bool Air::is_replaceable_by(const Base&) const
{
	return true;
}

}
