#include "../Event.hpp"

#include <cstdint>

class Event_window_size_change : public Event
{
	public:
		Event_window_size_change(uint_fast32_t width, uint_fast32_t height);

		uint_fast32_t width;
		uint_fast32_t height;
};
