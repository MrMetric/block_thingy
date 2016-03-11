#include "Event_window_size_change.hpp"

#include "../EventType.hpp"

Event_window_size_change::Event_window_size_change(const int width, const int height)
	:
	Event(EventType::window_size_change),
	width(width),
	height(height)
{
}


