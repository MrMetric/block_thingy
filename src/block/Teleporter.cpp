#include "Teleporter.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace Block {

Teleporter::Teleporter(const Enum::Type t)
:
	Base(t, Enum::VisibilityType::translucent, "teleporter")
{
}

bool Teleporter::is_solid() const
{
	return false;
}

}
