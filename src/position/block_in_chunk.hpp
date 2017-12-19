#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdint.h>

#ifdef DEBUG_BUILD
	#include <stdexcept>
	#include <string>
#endif

#include <glm/vec3.hpp>
#include <glm/gtx/io.hpp>

#include "fwd/position/block_in_world.hpp"

namespace block_thingy::position {

struct block_in_chunk
{
	using value_type = uint8_t;
	using vec_type = glm::tvec3<value_type>;

	block_in_chunk();
	block_in_chunk(value_type x, value_type y, value_type z);
	explicit block_in_chunk(const block_in_world&);

	value_type operator[](const std::ptrdiff_t i) const
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::block_in_chunk::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	value_type& operator[](const std::ptrdiff_t i)
	{
	#ifdef DEBUG_BUILD
		if(i > 2)
		{
			throw std::out_of_range("position::block_in_chunk::operator[]: " + std::to_string(i) + " > 2");
		}
	#endif
		return (&x)[i];
	}
	block_in_chunk& operator+=(const block_in_chunk&);
	bool operator==(const block_in_chunk&) const;
	bool operator!=(const block_in_chunk&) const;

	operator vec_type() const;

	value_type x, y, z;

private:
#ifdef DEBUG_BUILD
	void check_bounds();
#endif
};

std::ostream& operator<<(std::ostream&, const block_in_chunk&);

}

namespace glm {

template <typename CharT, typename Traits, precision P>
GLM_FUNC_QUALIFIER std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, tvec3<uint8_t, P> const& a)
{
	tvec3<unsigned int, P> b(a);
	return detail::print_vector_on(os, b);
}

}
