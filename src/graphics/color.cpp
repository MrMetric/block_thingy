#include "color.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

#include <glm/common.hpp>

namespace block_thingy::graphics {

color::value_type color::max = 16;

color::color()
:
	color(0, 0, 0)
{
}

color::color(value_type a)
:
	color(a, a, a)
{
}

color::color(value_type r, value_type g, value_type b)
:
	r(r),
	g(g),
	b(b)
{
}

color::color(const glm::dvec3& v)
:
	r(static_cast<value_type>(std::round(v.r * max))),
	g(static_cast<value_type>(std::round(v.g * max))),
	b(static_cast<value_type>(std::round(v.b * max)))
{
}

// defined for std::tuple hashing
bool color::operator<(const color& that) const
{
	int_fast32_t rgb1 = (r << 16) | (g << 8) | b;
	int_fast32_t rgb2 = (that.r << 16) | (that.g << 8) | that.b;
	return rgb1 < rgb2;
}

color color::operator+(const color& that) const
{
	return color
	(
		r + that.r,
		g + that.g,
		b + that.b
	);
}

color color::operator-(const color& that) const
{
	return color
	(
		r - that.r,
		g - that.g,
		b - that.b
	);
}

color color::operator+(const value_type x) const
{
	const int m = std::numeric_limits<value_type>::max();
	return color
	(
		static_cast<value_type>(std::min(r + x, m)),
		static_cast<value_type>(std::min(g + x, m)),
		static_cast<value_type>(std::min(b + x, m))
	);
}

color color::operator-(const value_type x) const
{
	return color
	(
		static_cast<value_type>(std::max(r - x, 0)),
		static_cast<value_type>(std::max(g - x, 0)),
		static_cast<value_type>(std::max(b - x, 0))
	);
}

color& color::operator-=(const value_type x)
{
	r = static_cast<value_type>(std::max(r - x, 0));
	g = static_cast<value_type>(std::max(g - x, 0));
	b = static_cast<value_type>(std::max(b - x, 0));
	return *this;
}

color::operator glm::vec3() const
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


color::operator glm::tvec3<uint8_t>() const
{
	return glm::tvec3<uint8_t>(glm::round(static_cast<glm::vec3>(*this) * 255.0f));
}

std::ostream& operator<<(std::ostream& o, const color& c)
{
	return o << '(' << +c.r << ',' << +c.g << ',' << +c.b << ')';
}

}
