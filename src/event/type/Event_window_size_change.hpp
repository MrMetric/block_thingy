#include "../Event.hpp"

class Event_window_size_change : public Event
{
	public:
		Event_window_size_change(int width, int height);

		int width;
		int height;
};
