#include "Color.hpp"

#include <cmath>
#include <iostream>

namespace Graphics {

Color::Color()
	:
	Color(0, 0, 0)
{
}

Color::Color(uint8_t r, uint8_t g, uint8_t b)
	:
	r(r),
	g(g),
	b(b)
{
}

glm::vec3 Color::to_vec3() const
{
	glm::vec3 v
	{
		std::fmin(1, r / 16.0f),
		std::fmin(1, g / 16.0f),
		std::fmin(1, b / 16.0f)
	};
	return v;
}

bool Color::operator==(const Color& that) const
{
	return (r == that.r) && (g == that.g) && (b == that.b);
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

bool Color::operator<(const int x) const
{
	return (r < x) && (g < x) && (b < x);
}

Color Color::operator+(const int x) const
{
	return Color(
		static_cast<decltype(r)>(std::min(r + x, 255)),
		static_cast<decltype(g)>(std::min(g + x, 255)),
		static_cast<decltype(b)>(std::min(b + x, 255))
	);
}

Color Color::operator-(const int x) const
{
	return Color(
		static_cast<decltype(r)>(std::max(r - x, 0)),
		static_cast<decltype(g)>(std::max(g - x, 0)),
		static_cast<decltype(b)>(std::max(b - x, 0))
	);
}

std::ostream& operator<<(std::ostream& o, const Color& c)
{
	return o << '('
			<< std::to_string(c.r) << ','
			<< std::to_string(c.g) << ','
			<< std::to_string(c.b) << ')';
}

} // namespace graphics
