#include "KnowsPosition.hpp"

namespace Block::Interface {

KnowsPosition::~KnowsPosition()
{
}

void KnowsPosition::set_position(Position::BlockInWorld pos)
{
	position = pos;
}

Position::BlockInWorld KnowsPosition::get_position() const
{
	return position;
}

}
