#include "Teleporter.hpp"

#include "block/Enum/VisibilityType.hpp"

namespace Block {

Teleporter::Teleporter(BlockType t)
:
	Base(t)
{
}

BlockVisibilityType Teleporter::visibility_type() const
{
	return BlockVisibilityType::translucent;
}

bool Teleporter::is_solid() const
{
	return false;
}

}
