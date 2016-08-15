#pragma once
#include "event/Event.hpp"

#include "fwd/Player.hpp"
#include "fwd/block/Block.hpp"

class Event_enter_block : public Event
{
	public:
		Event_enter_block(Player&, const Block::Block&);

		Player& player;
		const Block::Block& block;
};
