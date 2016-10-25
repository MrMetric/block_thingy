#include "None.hpp"

#include "block/BlockType.hpp"
#include "block/BlockVisibilityType.hpp"

namespace Block {

None::None(BlockType t)
:
	Base(t)
{
}

glm::dvec4 None::selection_color() const
{
	return {1, 0, 0, 1};
}

BlockVisibilityType None::visibility_type() const
{
	return BlockVisibilityType::invisible;
}

}
