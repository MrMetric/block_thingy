#include "Event_change_setting.hpp"

#include "event/EventType.hpp"

namespace block_thingy {

Event_change_setting::Event_change_setting
(
	std::string name,
	settings::value_t old_value,
	settings::value_t new_value
)
:
	Event(EventType::change_setting),
	name(std::move(name)),
	old_value(std::move(old_value)),
	new_value(std::move(new_value))
{
}

}
