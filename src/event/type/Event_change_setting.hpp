#pragma once
#include "event/Event.hpp"

#include <string>

#include "settings.hpp"

namespace block_thingy {

class Event_change_setting : public Event
{
public:
	Event_change_setting
	(
		std::string name,
		settings::value_t old_value,
		settings::value_t new_value
	);

	const std::string name;
	const settings::value_t old_value;
	const settings::value_t new_value;
};

}
