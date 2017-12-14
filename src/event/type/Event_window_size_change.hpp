#pragma once
#include "event/Event.hpp"

#include "types/window_size_t.hpp"

namespace block_thingy {

class Event_window_size_change : public Event
{
public:
	Event_window_size_change
	(
		const window_size_t&
	);

	const window_size_t window_size;
};

}
