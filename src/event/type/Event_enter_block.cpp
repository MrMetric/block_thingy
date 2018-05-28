#include "Event_enter_block.hpp"

#include "event/EventType.hpp"

namespace block_thingy {

Event_enter_block::Event_enter_block
(
	world::world& world,
	const position::block_in_world& position,
	const block_t block,
	Player& player
)
:
	Event(EventType::enter_block),
	world(world),
	position(position),
	block(block),
	player(player)
{
}

}
