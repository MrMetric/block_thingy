#include "Event.hpp"

Event::Event(EventType type)
	:
	type_(type)
{
}

EventType Event::type() const
{
	return type_;
}
