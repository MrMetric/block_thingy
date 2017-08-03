#include "Event_change_setting.hpp"

#include "event/EventType.hpp"

Event_change_setting::Event_change_setting
(
	const std::string& name,
	const value_t value
)
:
	Event(EventType::change_setting),
	name(name),
	value(value)
{
}
