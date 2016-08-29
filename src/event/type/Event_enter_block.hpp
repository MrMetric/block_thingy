#pragma once
#include "event/Event.hpp"

#include "fwd/Player.hpp"
#include "fwd/block/Base.hpp"

class Event_enter_block : public Event
{
	public:
		Event_enter_block(Player&, const Block::Base&);

		Player& player;
		const Block::Base& block;
};
