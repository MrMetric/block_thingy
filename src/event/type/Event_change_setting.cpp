#include "Event_change_setting.hpp"

#include "event/EventType.hpp"

Event_change_setting::Event_change_setting
(
	const std::string& name,
	const value_t old_value,
	const value_t new_value
)
:
	Event(EventType::change_setting),
	name(name),
	old_value(old_value),
	new_value(new_value)
{
}
