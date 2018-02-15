#pragma once

#include <functional>

namespace block_thingy::util {

template<typename T>
class property
{
public:
	property(T value)
	:
		value(value),
		set([](T){})
	{
	}

	property
	(
		T value,
		std::function<void(T)> set
	)
	:
		value(value),
		set(set)
	{
	}

	property(property&&) = delete;
	property(const property&) = delete;
	property& operator=(property&&) = delete;
	property& operator=(const property&) = delete;

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

}
