#pragma once

#include <cmath>
#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>

namespace Graphics {

struct Color
{
	using value_type = uint8_t;
	static value_type max;

	Color();
	Color(value_type r, value_type g, value_type b);

	glm::vec3 to_vec3() const;

	bool operator==(const Color&) const;
	bool operator<(const Color&) const;
	Color operator+(const Color&) const;
	Color operator-(const Color&) const;

	bool operator<(value_type) const;
	Color operator+(value_type) const;
	Color operator-(value_type) const;
	Color& operator-=(value_type);

	value_type r;
	value_type g;
	value_type b;
};

std::ostream& operator<<(std::ostream&, const Color&);

} // namespace graphics
