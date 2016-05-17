#include "Event_enter_block.hpp"

#include "event/EventType.hpp"

Event_enter_block::Event_enter_block(Player& player, const Block::Block& block)
	:
	Event(EventType::enter_block),
	player(player),
	block(block)
{
}
