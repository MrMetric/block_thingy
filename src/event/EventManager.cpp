#include "EventManager.hpp"

#include <stdexcept>
#include <string>
#include <utility>

#include "Event.hpp"
#include "EventType.hpp"

using std::to_string;

EventManager::EventManager()
:
	max_id(0)
{
}

event_handler_id_t EventManager::add_handler(const event_handler_t& handler)
{
	return add_handler(EventType::any, handler);
}

event_handler_id_t EventManager::add_handler(const EventType type, const event_handler_t& handler)
{
	auto id = max_id++;
	handlers[id] = {type, handler};
	return id;
}

void EventManager::unadd_handler(const event_handler_id_t event_id)
{
	const auto i = handlers.find(event_id);
	if(i == handlers.cend())
	{
		throw std::runtime_error("there is no event with id " + to_string(event_id));
	}
	handlers.erase(i);
}

void EventManager::do_event(const Event& event) const
{
	for(auto pair0 : handlers)
	{
		std::pair<EventType, event_handler_t> pair1 = pair0.second;
		if(pair1.first == event.type() || pair1.first == EventType::any)
		{
			pair1.second(event);
		}
	}
}
