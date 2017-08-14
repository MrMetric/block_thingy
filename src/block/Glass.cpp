#include "Glass.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace Block {

Glass::Glass(const Enum::Type t)
:
	SimpleShader(t, "glass")
{
}

Enum::VisibilityType Glass::visibility_type() const
{
	return Enum::VisibilityType::translucent;
}

}
