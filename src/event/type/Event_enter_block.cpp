#include "Event_enter_block.hpp"

#include "event/EventType.hpp"

using std::shared_ptr;

Event_enter_block::Event_enter_block
(
	World& world,
	Player& player,
	const shared_ptr<Block::Base> block
)
:
	Event(EventType::enter_block),
	world(world),
	player(player),
	block(block)
{
}
