#pragma once
#include "event/Event.hpp"

#include <string>

#include "Settings.hpp"

class Event_change_setting : public Event
{
public:
	Event_change_setting
	(
		const std::string& name,
		const Settings::value_t old_value,
		const Settings::value_t new_value
	);

	const std::string name;
	const Settings::value_t old_value;
	const Settings::value_t new_value;
};
