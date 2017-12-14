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

struct Color
{
	using value_type = uint8_t;
	static value_type max;

	Color();
	Color(value_type);
	Color(value_type r, value_type g, value_type b);
	Color(const glm::dvec3&);

	value_type operator[](const std::ptrdiff_t i) const
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("graphics::Color::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&r)[i];
	}

	value_type& operator[](const std::ptrdiff_t i)
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("graphics::Color::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&r)[i];
	}

	bool operator==(const Color& that) const
	{
		return (r == that.r) && (g == that.g) && (b == that.b);
	}
	bool operator!=(const Color& that) const
	{
		return (r != that.r) || (g != that.g) || (b != that.b);
	}
	bool operator<(const Color&) const; // defined for std::tuple hashing

	Color operator+(const Color&) const;
	Color operator-(const Color&) const;

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

	Color operator+(value_type) const;
	Color operator-(value_type) const;
	Color& operator-=(value_type);

	operator glm::vec3() const;
	operator glm::tvec3<uint8_t>() const;

	value_type r;
	value_type g;
	value_type b;
};

std::ostream& operator<<(std::ostream&, const Color&);

}
