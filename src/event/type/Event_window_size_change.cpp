#include "Event_window_size_change.hpp"

#include "../EventType.hpp"

Event_window_size_change::Event_window_size_change(const uint_fast32_t width, const uint_fast32_t height)
	:
	Event(EventType::window_size_change),
	width(width),
	height(height)
{
}


