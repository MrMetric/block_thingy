#pragma once

#include "EventType.hpp"

class Event
{
	public:
		Event(EventType);

		EventType type() const;

	private:
		EventType type_;
};