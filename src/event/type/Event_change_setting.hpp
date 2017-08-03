#pragma once
#include "event/Event.hpp"

#include <stdint.h>
#include <string>

#include <strict_variant/variant.hpp>

class Event_change_setting : public Event
{
public:
	using value_t = strict_variant::variant<bool, double, int64_t, std::string>;

	Event_change_setting
	(
		const std::string& name,
		const value_t value
	);

	const std::string name;
	const value_t value;
};
