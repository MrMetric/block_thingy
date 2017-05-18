#include "Glass.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace Block {

Glass::Glass(Enum::Type t)
:
	Base(t)
{
}

Enum::VisibilityType Glass::visibility_type() const
{
	return Enum::VisibilityType::translucent;
}

}
