#include "Teleporter.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace block_thingy::block {

Teleporter::Teleporter(const enums::Type t)
:
	Base(t, enums::VisibilityType::translucent, "teleporter")
{
}

bool Teleporter::is_solid() const
{
	return false;
}

}
