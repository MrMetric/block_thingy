#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdint.h>

#ifdef DEBUG_BUILD
	#include <stdexcept>
	#include <string>
#endif

#include <glm/vec3.hpp>

#include "fwd/position/block_in_world.hpp"

namespace block_thingy::position {

struct chunk_in_world
{
	using value_type = int64_t;
	using vec_type = glm::tvec3<value_type>;

	chunk_in_world();
	chunk_in_world(value_type x, value_type y, value_type z);
	explicit chunk_in_world(const block_in_world&);

	value_type operator[](const std::ptrdiff_t i) const
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::chunk_in_world::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	value_type& operator[](const std::ptrdiff_t i)
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::chunk_in_world::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	chunk_in_world& operator+=(const chunk_in_world&);
	bool operator==(const chunk_in_world&) const;
	bool operator!=(const chunk_in_world&) const;

	operator vec_type() const;

	value_type x, y, z;
};

chunk_in_world operator-(const chunk_in_world&, chunk_in_world::value_type);
chunk_in_world operator+(const chunk_in_world&, chunk_in_world::value_type);
chunk_in_world operator*(const chunk_in_world&, chunk_in_world::value_type);

chunk_in_world operator-(const chunk_in_world&, const chunk_in_world&);
chunk_in_world operator+(const chunk_in_world&, const chunk_in_world&);

std::ostream& operator<<(std::ostream&, const chunk_in_world&);

}
