#include "Teleporter.hpp"

#include "block/BlockVisibilityType.hpp"

namespace Block {

Teleporter::Teleporter()
:
	Base(BlockType::teleporter)
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
