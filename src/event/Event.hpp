#pragma once

enum class EventType;

class Event
{
	public:
		explicit Event(EventType);

		EventType type() const;

	private:
		EventType type_;
};
