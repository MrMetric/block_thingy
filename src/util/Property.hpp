#pragma once

#include <functional>

template<typename T>
class Property
{
public:
	Property
	(
		T value,
		std::function<void(T)> set
	)
	:
		value(value),
		set(set)
	{
	}

	Property(Property&&) = delete;
	Property(const Property&) = delete;
	void operator=(const Property&) = delete;

	T operator()() const
	{
		return value;
	}

	T operator=(T new_value)
	{
		set(new_value);
		return value = new_value;
	}

private:
	T value;
	std::function<void(T)> set;
};
