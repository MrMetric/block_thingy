#pragma once
#include "event/Event.hpp"

class Player;
namespace Block
{
	class Block;
}

class Event_enter_block : public Event
{
	public:
		Event_enter_block(Player&, const Block::Block&);

		Player& player;
		const Block::Block& block;
};
