#pragma once

#include "fwd/event/EventType.hpp"

class Event
{
public:
	explicit Event(EventType);

	Event(Event&&) = delete;
	Event(const Event&) = delete;
	void operator=(Event&&) = delete;
	void operator=(const Event&) = delete;

	EventType type() const;

private:
	EventType type_;
};
