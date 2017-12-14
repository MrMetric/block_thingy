#include "Air.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace block_thingy::block {

Air::Air(const enums::Type t)
:
	Base(t, enums::VisibilityType::invisible)
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
