#include "none.hpp"

#include "block/enums/visibility_type.hpp"

namespace block_thingy::block {

none::none(const enums::type t)
:
	base(t, enums::visibility_type::invisible)
{
}

glm::dvec4 none::selection_color() const
{
	return {1, 0, 0, 1};
}

}
