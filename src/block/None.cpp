#include "None.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace block_thingy::block {

None::None(const enums::Type t)
:
	Base(t, enums::VisibilityType::invisible)
{
}

glm::dvec4 None::selection_color() const
{
	return {1, 0, 0, 1};
}

}
