#include "Color.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using std::to_string;

namespace Graphics {

Color::value_type Color::max = 16;

Color::Color()
	:
	Color(0, 0, 0)
{
}

Color::Color(value_type r, value_type g, value_type b)
	:
	r(r),
	g(g),
	b(b)
{
}

Color::value_type Color::operator[](const uint_fast8_t i) const
{
	if(i == 0) return r;
	if(i == 1) return g;
	if(i == 2) return b;
	throw std::out_of_range("Graphics::Color::operator[]: " + to_string(i) + " > 2");
}

Color::value_type& Color::operator[](const uint_fast8_t i)
{
	if(i == 0) return r;
	if(i == 1) return g;
	if(i == 2) return b;
	throw std::out_of_range("Graphics::Color::operator[]: " + to_string(i) + " > 2");
}

bool Color::operator==(const Color& that) const
{
	return (r == that.r) && (g == that.g) && (b == that.b);
}

bool Color::operator!=(const Color& that) const
{
	return (r != that.r) || (g != that.g) || (b != that.b);
}

// defined for std::tuple hashing
bool Color::operator<(const Color& that) const
{
	int_fast32_t rgb1 = (r << 16) | (g << 8) | b;
	int_fast32_t rgb2 = (that.r << 16) | (that.g << 8) | that.b;
	return rgb1 < rgb2;
}

Color Color::operator+(const Color& that) const
{
	return Color(
		r + that.r,
		g + that.g,
		b + that.b
	);
}

Color Color::operator-(const Color& that) const
{
	return Color(
		r - that.r,
		g - that.g,
		b - that.b
	);
}

bool Color::operator==(const value_type x) const
{
	return (r == x) && (g == x) && (b == x);
}

bool Color::operator!=(const value_type x) const
{
	return (r != x) || (g != x) || (b != x);
}

bool Color::operator<(const value_type x) const
{
	return (r < x) && (g < x) && (b < x);
}

Color Color::operator+(const value_type x) const
{
	const int m = std::numeric_limits<value_type>::max();
	return Color(
		static_cast<value_type>(std::min(r + x, m)),
		static_cast<value_type>(std::min(g + x, m)),
		static_cast<value_type>(std::min(b + x, m))
	);
}

Color Color::operator-(const value_type x) const
{
	return Color(
		static_cast<value_type>(std::max(r - x, 0)),
		static_cast<value_type>(std::max(g - x, 0)),
		static_cast<value_type>(std::max(b - x, 0))
	);
}

Color& Color::operator-=(const value_type x)
{
	r = static_cast<value_type>(std::max(r - x, 0));
	g = static_cast<value_type>(std::max(g - x, 0));
	b = static_cast<value_type>(std::max(b - x, 0));
	return *this;
}

Color::operator glm::vec3() const
{
	const float m = max;
	glm::vec3 v
	{
		std::fmin(1, r / m),
		std::fmin(1, g / m),
		std::fmin(1, b / m),
	};
	return v;
}

std::ostream& operator<<(std::ostream& o, const Color& c)
{
	return o << '('
			<< std::to_string(c.r) << ','
			<< std::to_string(c.g) << ','
			<< std::to_string(c.b) << ')';
}

} // namespace graphics
