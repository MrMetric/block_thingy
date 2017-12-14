#include "Event.hpp"

namespace block_thingy {

Event::Event(EventType type)
:
	type_(type)
{
}

EventType Event::type() const
{
	return type_;
}

}
