#pragma once

#include "fwd/event/EventType.hpp"

namespace block_thingy {

class Event
{
public:
	explicit Event(EventType);

	Event(Event&&) = delete;
	Event(const Event&) = delete;
	Event& operator=(Event&&) = delete;
	Event& operator=(const Event&) = delete;

	EventType type() const;

private:
	EventType type_;
};

}
