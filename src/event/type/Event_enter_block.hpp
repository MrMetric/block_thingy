#pragma once
#include "event/Event.hpp"

#include "fwd/Player.hpp"
#include "block/block.hpp"
#include "position/block_in_world.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy {

class Event_enter_block : public Event
{
public:
	Event_enter_block
	(
		world::world&,
		const position::block_in_world&,
		block_t,
		Player&
	);

	world::world& world;
	position::block_in_world position;
	block_t block;
	Player& player;
};

}
