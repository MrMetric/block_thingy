#pragma once

#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>

namespace Graphics {

struct Color
{
	using value_type = uint8_t;
	static value_type max;

	Color();
	Color(value_type);
	Color(value_type r, value_type g, value_type b);
	Color(const glm::dvec3&);

	value_type operator[](uint_fast8_t) const;
	value_type& operator[](uint_fast8_t);

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

} // namespace graphics
