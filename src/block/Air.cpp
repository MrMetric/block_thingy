#include "Air.hpp"

#include "block/BlockType.hpp"
#include "block/BlockVisibilityType.hpp"

namespace Block {

Air::Air()
	:
	Block(BlockType::air)
{
}

BlockVisibilityType Air::visibility_type() const
{
	return BlockVisibilityType::invisible;
}

bool Air::is_solid() const
{
	return false;
}

bool Air::is_selectable() const
{
	return false;
}

} // namespace Block
