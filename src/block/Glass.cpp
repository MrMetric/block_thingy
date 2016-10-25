#include "Glass.hpp"

#include "block/BlockVisibilityType.hpp"

namespace Block {

Glass::Glass(BlockType t)
:
	Base(t)
{
}

BlockVisibilityType Glass::visibility_type() const
{
	return BlockVisibilityType::translucent;
}

}
