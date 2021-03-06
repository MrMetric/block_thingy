#include "EventManager.hpp"

#include <stdexcept>
#include <string>
#include <utility>

#include "event/Event.hpp"
#include "event/EventType.hpp"

namespace block_thingy {

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
	std::lock_guard<std::mutex> g(mutex);
	const auto id = max_id++;
	handlers[id] = {type, handler};
	return id;
}

void EventManager::unadd_handler(const event_handler_id_t event_id)
{
	std::lock_guard<std::mutex> g(mutex);
	const auto i = handlers.find(event_id);
	if(i == handlers.cend())
	{
		throw std::runtime_error("there is no event with id " + std::to_string(event_id));
	}
	handlers.erase(i);
}

void EventManager::do_event(const Event& event) const
{
	// TODO: avoid deadlocking when triggering an event from an event handler; perhaps an event queue would be useful
	std::lock_guard<std::mutex> g(mutex);
	for(auto pair0 : handlers)
	{
		std::pair<EventType, event_handler_t> pair1 = pair0.second;
		if(pair1.first == event.type() || pair1.first == EventType::any)
		{
			pair1.second(event);
		}
	}
}

}
