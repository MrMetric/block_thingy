#pragma once
#include "event/Event.hpp"

#include <string>

class Event_change_setting : public Event
{
public:
	Event_change_setting
	(
		const std::string& name,
		const void* value
	);

	const std::string name;
	const void* value; // not safe, but I dunno how else to do it right now
};
