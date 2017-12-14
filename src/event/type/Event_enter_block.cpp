#include "Event_enter_block.hpp"

#include "event/EventType.hpp"

using std::shared_ptr;

namespace block_thingy {

Event_enter_block::Event_enter_block
(
	World& world,
	Player& player,
	const shared_ptr<block::Base> block
)
:
	Event(EventType::enter_block),
	world(world),
	player(player),
	block(block)
{
}

}
