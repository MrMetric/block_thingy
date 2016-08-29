#include "None.hpp"

#include "block/BlockType.hpp"
#include "block/BlockVisibilityType.hpp"

namespace Block {

None::None()
	:
	Block(BlockType::none)
{
}

BlockVisibilityType None::visibility_type() const
{
	return BlockVisibilityType::invisible;
}

} // namespace Block
