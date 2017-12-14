#include "Event_change_setting.hpp"

#include "event/EventType.hpp"

namespace block_thingy {

Event_change_setting::Event_change_setting
(
	const std::string& name,
	const settings::value_t old_value,
	const settings::value_t new_value
)
:
	Event(EventType::change_setting),
	name(name),
	old_value(old_value),
	new_value(new_value)
{
}

}
