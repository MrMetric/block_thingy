#pragma once

#include <cmath>
#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>

namespace Graphics {

struct Color
{
	Color();
	Color(uint8_t r, uint8_t g, uint8_t b);

	glm::vec3 to_vec3() const;

	bool operator==(const Color&) const;
	bool operator<(const Color&) const;
	Color operator+(const Color&) const;
	Color operator-(const Color&) const;

	bool operator<(int) const;
	Color operator+(int) const;
	Color operator-(int) const;
	Color& operator-=(int);

	uint8_t r;
	uint8_t g;
	uint8_t b;
};

std::ostream& operator<<(std::ostream&, const Color&);

} // namespace graphics
