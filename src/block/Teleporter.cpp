#include "Teleporter.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace Block {

Teleporter::Teleporter(const Enum::Type t)
:
	SimpleShader(t, "teleporter")
{
}

Enum::VisibilityType Teleporter::visibility_type() const
{
	return Enum::VisibilityType::translucent;
}

bool Teleporter::is_solid() const
{
	return false;
}

}
