#pragma once

#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>
#include <glm/gtx/io.hpp>

#include "fwd/position/BlockInWorld.hpp"

namespace Position {

struct BlockInChunk
{
	using value_type = uint8_t;
	using vec_type = glm::tvec3<value_type>;

	BlockInChunk();
	BlockInChunk(value_type x, value_type y, value_type z);
	explicit BlockInChunk(const BlockInWorld&);

	value_type operator[](uint_fast8_t) const;
	value_type& operator[](uint_fast8_t);
	BlockInChunk& operator+=(const BlockInChunk&);
	bool operator==(const BlockInChunk&) const;
	bool operator!=(const BlockInChunk&) const;

	operator vec_type() const;

	value_type x, y, z;

private:
	void check_bounds();
};

std::ostream& operator<<(std::ostream&, const BlockInChunk&);

}

namespace glm {

template <typename CharT, typename Traits, precision P>
GLM_FUNC_QUALIFIER std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, tvec3<uint8_t, P> const& a)
{
	tvec3<uint_fast16_t, P> b(a);
	return detail::print_vector_on(os, b);
}

}