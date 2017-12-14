#pragma once
#include "event/Event.hpp"

#include <memory>

#include "fwd/Player.hpp"
#include "fwd/World.hpp"
#include "fwd/block/Base.hpp"

namespace block_thingy {

class Event_enter_block : public Event
{
public:
	Event_enter_block
	(
		World&,
		Player&,
		std::shared_ptr<block::Base>
	);

	World& world;
	Player& player;
	std::shared_ptr<block::Base> block;
};

}
