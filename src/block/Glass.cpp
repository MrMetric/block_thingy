#include "Glass.hpp"

#include "block/BlockVisibilityType.hpp"

namespace Block {

Glass::Glass()
:
	Base(BlockType::glass)
{
}

BlockVisibilityType Glass::visibility_type() const
{
	return BlockVisibilityType::translucent;
}

}
