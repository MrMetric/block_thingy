#pragma once

#include <functional>

template <typename T> class Property
{
	public:
		Property(
			T value,
			std::function<void(T)> set
		)
			:
			value(value),
			set(set)
		{
		}

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
