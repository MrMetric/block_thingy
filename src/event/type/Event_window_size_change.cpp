#include "Event_window_size_change.hpp"

#include "event/EventType.hpp"

namespace block_thingy {

Event_window_size_change::Event_window_size_change
(
	const window_size_t& window_size
)
:
	Event(EventType::window_size_change),
	window_size(window_size)
{
}

}
