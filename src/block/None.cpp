#include "None.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace Block {

None::None(const Enum::Type t)
:
	Base(t, Enum::VisibilityType::invisible)
{
}

glm::dvec4 None::selection_color() const
{
	return {1, 0, 0, 1};
}

}
