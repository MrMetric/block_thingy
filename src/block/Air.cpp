#include "Air.hpp"

#include "block/Enum/Type.hpp"
#include "block/Enum/VisibilityType.hpp"

namespace Block {

Air::Air(BlockType t)
:
	Base(t)
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

bool Air::is_replaceable() const
{
	return true;
}

}
