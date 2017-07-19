#pragma once

#include "fwd/event/EventType.hpp"

class Event
{
public:
	explicit Event(EventType);

	EventType type() const;

private:
	EventType type_;
};
