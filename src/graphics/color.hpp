#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdint.h>

#ifdef DEBUG_BUILD
	#include <stdexcept>
	#include <string>
#endif

#include <glm/vec3.hpp>

namespace block_thingy::graphics {

struct color
{
	using value_type = uint8_t;
	static value_type max;

	color();
	color(value_type);
	color(value_type r, value_type g, value_type b);
	color(const glm::dvec3&);

	value_type operator[](const std::ptrdiff_t i) const
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("graphics::color::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&r)[i];
	}

	value_type& operator[](const std::ptrdiff_t i)
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("graphics::color::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&r)[i];
	}

	bool operator==(const color& that) const
	{
		return (r == that.r) && (g == that.g) && (b == that.b);
	}
	bool operator!=(const color& that) const
	{
		return (r != that.r) || (g != that.g) || (b != that.b);
	}
	bool operator<(const color&) const; // defined for std::tuple hashing

	color operator+(const color&) const;
	color operator-(const color&) const;

	bool operator==(const value_type x) const
	{
		return (r == x) && (g == x) && (b == x);
	}
	bool operator!=(const value_type x) const
	{
		return (r != x) || (g != x) || (b != x);
	}
	bool operator<(const value_type x) const
	{
		return (r < x) && (g < x) && (b < x);
	}
	bool operator>(const value_type x) const
	{
		return (r > x) && (g > x) && (b > x);
	}

	color operator+(value_type) const;
	color operator-(value_type) const;
	color& operator-=(value_type);

	operator glm::vec3() const;
	operator glm::tvec3<uint8_t>() const;

	value_type r;
	value_type g;
	value_type b;
};

std::ostream& operator<<(std::ostream&, const color&);

}
