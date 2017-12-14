#include "Color.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

#include <glm/common.hpp>

namespace block_thingy::graphics {

Color::value_type Color::max = 16;

Color::Color()
:
	Color(0, 0, 0)
{
}

Color::Color(value_type a)
:
	Color(a, a, a)
{
}

Color::Color(value_type r, value_type g, value_type b)
:
	r(r),
	g(g),
	b(b)
{
}

Color::Color(const glm::dvec3& v)
:
	r(static_cast<value_type>(std::round(v.r * max))),
	g(static_cast<value_type>(std::round(v.g * max))),
	b(static_cast<value_type>(std::round(v.b * max)))
{
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
	return Color
	(
		r + that.r,
		g + that.g,
		b + that.b
	);
}

Color Color::operator-(const Color& that) const
{
	return Color
	(
		r - that.r,
		g - that.g,
		b - that.b
	);
}

Color Color::operator+(const value_type x) const
{
	const int m = std::numeric_limits<value_type>::max();
	return Color
	(
		static_cast<value_type>(std::min(r + x, m)),
		static_cast<value_type>(std::min(g + x, m)),
		static_cast<value_type>(std::min(b + x, m))
	);
}

Color Color::operator-(const value_type x) const
{
	return Color
	(
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
		std::min(1.0f, r / m),
		std::min(1.0f, g / m),
		std::min(1.0f, b / m),
	};
	return v;
}


Color::operator glm::tvec3<uint8_t>() const
{
	return glm::tvec3<uint8_t>(glm::round(static_cast<glm::vec3>(*this) * 255.0f));
}

std::ostream& operator<<(std::ostream& o, const Color& c)
{
	return o << '(' << +c.r << ',' << +c.g << ',' << +c.b << ')';
}

}
