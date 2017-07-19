#pragma once

#include "position/BlockInWorld.hpp"

namespace Block::Interface {

class KnowsPosition
{
public:
	virtual ~KnowsPosition();

	virtual void set_position(Position::BlockInWorld);
	Position::BlockInWorld get_position() const;

protected:
	Position::BlockInWorld position;
};

}
